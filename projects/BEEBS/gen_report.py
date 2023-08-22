import json
from scipy.stats import gmean

reports = [
    "report_baseline.json",
    "report_asan.json",
    "report_usan.json",
    # "report_usan_2k.json",
    # "report_usan_4k.json",
    # "report_usan_8k.json"
]

output = dict()

for report_file in reports:
    with open(report_file, "r") as f:
        data = json.load(f)
        for bm in data:
            bm_name = bm["benchmark"][:-4]
            if bm_name not in output:
                output[bm_name] = {"benchmark": bm_name}
            if report_file == "report_baseline.json":
                output[bm_name]["baseline"] = bm["time"]
            elif report_file == "report_asan.json":
                output[bm_name]["asan"] = bm["time"] / output[bm_name]["baseline"]
            elif report_file == "report_usan.json":
                output[bm_name]["usan"] = bm["time"] / output[bm_name]["baseline"]
            # elif report_file == "report_usan_2k.json":
            #     output[bm_name]["usan_2k"] = bm["time"] / output[bm_name]["baseline"]
            # elif report_file == "report_usan_4k.json":
            #     output[bm_name]["usan_4k"] = bm["time"] / output[bm_name]["baseline"]
            # elif report_file == "report_usan_8k.json":
            #     output[bm_name]["usan_8k"] = bm["time"] / output[bm_name]["baseline"]
            else:
                assert False, "Unknown report %s" % report_file

beebs_raw_data = list(output.values())
# beebs_data.sort(key=lambda x: x["benchmark"])
data_present = lambda x, r: x in r and r[x] > 0
beebs_data = list()
for d in beebs_raw_data:
    if data_present("asan", d) and data_present("usan", d):
        beebs_data.append(d)
beebs_data.sort(key=lambda x: x['benchmark'])

asan = [d["asan"] for d in beebs_data]
usan = [d["usan_1k"] for d in beebs_data]
# usan_2k = [d["usan_2k"] for d in beebs_data]
# usan_4k = [d["usan_4k"] for d in beebs_data]
# usan_8k = [d["usan_8k"] for d in beebs_data]

content = list()
content.append("\\begin{table}[t!]")
content.append("\t\\centering")
content.append("\t\\caption{BEEBS Results (normalized, the higher, the worse)}")
content.append("\t\\label{eval:table:beebs}")
content.append("\t\\begin{adjustbox}{max width=\columnwidth}")
content.append("\t\t\\begin{tabular}{rrrr}")
content.append("\t\t\t\\toprule")
content.append("\t\t\t& \\tabincell{c}{\\textbf{Baseline} \\\\ \\textbf{(ms)}} &")
content.append("\t\t\t\\tabincell{c}{\\textbf{ASan} \\\\ ($\\times$)} &")
content.append("\t\t\t\\tabincell{c}{\\sys (1k) \\\\ ($\\times$)} \\\\")

content.append("\t\t\t\\midrule")
for r in beebs_data:
    content.append(f"\t\t\t{r['benchmark']} & {r['baseline']:,d} & {r['asan']:.3f} & {r['usan']:.3f} \\\\")
content.append("\t\t\t\\midrule")
content.append("\t\t\t\\textbf{Min} & - & {%.3f} & {%.3f} \\\\" % (min(asan), min(usan)))
content.append("\t\t\t\\textbf{Max} & - & {%.3f} & {%.3f} \\\\" % (max(asan), max(usan)))
content.append("\t\t\t\\textbf{Geomean} & - & {%.3f} & {%.3f} \\\\" % (gmean(asan), gmean(usan)))
content.append("\t\t\t\\bottomrule")
content.append("\t\t\\end{tabular}")
content.append("\t\\end{adjustbox}")
content.append("\\end{table}")

print('\n'.join(content))

