#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <openssl/ssl.h>
#include <openssl/ct.h>
#include <openssl/ocsp.h>
#include <openssl/bio.h>
#include <openssl/stack.h>
#include <openssl/safestack.h>
#include "openssl_h/ocsp_lcl.h"

#define CIPHER_LIST  "TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256"

int (*openssl_SSL_do_handshake)(SSL *ssl);
int SSL_set_ciphersuites(SSL *s, const char *str);
static int ocsp_resp_cb(SSL *ssl, void *arg);
static void print_SCT_LIST(SSL *ssl);

void shim_init() {
    void *handle = dlopen("/usr/lib/libssl.so.1.1", RTLD_LAZY);
    
    if (!handle) {
        fprintf(stderr, "%s,\n", dlerror());
        exit(1);
    }

    openssl_SSL_do_handshake = dlsym(handle, "SSL_do_handshake");
    if (dlerror() != NULL) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }
}

int SSL_do_handshake(SSL *ssl) {
    shim_init();

    int ret;
    int is_server = SSL_is_server(ssl);
    const char *ciphers = CIPHER_LIST;

    // when ssl->server is 0, we are connecting to a server
    if (!is_server) {
        // set handshake to request SCTs
        SSL_enable_ct(ssl, SSL_CT_VALIDATION_STRICT);

        /* specify log list
        NOTE: this path will change depending on the location of the user's OpenSSL source */
        SSL_CTX_set_ctlog_list_file(
            SSL_get_SSL_CTX(ssl),
            "/usr/local/src/openssl-1.1.1a/apps/ct_log_list.cnf");

        // set OCSP callback
        SSL_CTX_set_tlsext_status_cb(SSL_get_SSL_CTX(ssl), ocsp_resp_cb);

        // set TLS cipher suites
        SSL_set_ciphersuites(ssl, ciphers);
    }

    ret = openssl_SSL_do_handshake(ssl);
    
    /* Uncomment to print SCT list for debugging purposes
    if (!is_server) {
        print_SCT_LIST(ssl);
    }
    */

    return(ret);
}

// callback that will check OCSP response to ensure certificate has not been revoked
// TODO: in case of no OCSP response, conduct CRL checks
static int ocsp_resp_cb(SSL *ssl, void *arg) {
    const unsigned char *holder;
    int i, len;
    OCSP_RESPONSE *response;
    OCSP_BASICRESP *br;
    OCSP_RESPDATA *rd;
    OCSP_SINGLERESP *single_resp;
    OCSP_CERTSTATUS *cst;

    // get raw OCSP response data
    len = SSL_get_tlsext_status_ocsp_resp(ssl, &holder);

    // if no response was given
    // TODO: conduct CRL check (ideally find way to parse CRLSets to OpenSSL readable format)
    if (!holder) {
        //fprintf(stderr, "NO OCSP\n");
        return 1;
    }
    
    // convert to OCSP response structure
    response = d2i_OCSP_RESPONSE(NULL, &holder, len);

    // check for parse errors
    if (!response) {
        //fprintf(stderr, "OCSP PARSE ERROR\n");
        return -1;
    }

    // determine cert status from OCSP resposne
    br = OCSP_response_get1_basic(response);
    rd = &br->tbsResponseData;

    for (i = 0; i < sk_OCSP_SINGLERESP_num(rd->responses); i++) {
        if (!sk_OCSP_SINGLERESP_value(rd->responses, i))
            continue;
        
        single_resp = sk_OCSP_SINGLERESP_value(rd->responses, i);
        cst = single_resp->certStatus;

        // if cerStatus is not 0 (good), certificate is unsafe/revoked
        if (cst->type) {
            //fprintf(stderr, "OCSP Cert Status invalid: %d\n", cst->type);
            return 0;
        }
    }

    OCSP_RESPONSE_free(response);
    return 1;
}

static void print_SCT_LIST(SSL *ssl) {
    const STACK_OF(SCT) *sct_list = SSL_get0_peer_scts(ssl);
    BIO *out = BIO_new_fp(stderr, BIO_NOCLOSE);

    SCT_LIST_print(sct_list, out, 0, "\n", NULL);
}
