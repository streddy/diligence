from collections import defaultdict

SHIM_FILE = '/home/streddy/diligence/handshake_latency/shim'
NO_SHIM_FILE = '/home/streddy/diligence/handshake_latency/no_shim'

shim_lines = [float(line.rstrip('\n')) for line in open(SHIM_FILE)]
no_shim_lines = [float(line.rstrip('\n')) for line in open(NO_SHIM_FILE)]

zipped = list(zip(shim_lines, no_shim_lines));

site_latencies = defaultdict(list)
for i, val in enumerate(zipped):
    site_latencies[i % 7].append(val)

site_avg = {}
for i, site_list in site_latencies.items():
    shim_total = 0
    no_shim_total = 0

    for (x, y) in site_list:
        shim_total += x
        no_shim_total += y

    site_avg[i] = (float(shim_total/100.0), float(no_shim_total/100.0))

total = 0
for i, (x, y) in site_avg.items():
    total += (x - y)

total /= 7

print(site_avg)
print(total)
