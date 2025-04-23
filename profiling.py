import argparse
from matplotlib.ticker import ScalarFormatter
import re
import pandas as pd
import matplotlib.pyplot as plt
import os
import shutil
import subprocess
import itertools
from itertools import product

# GRID SEARCH ON EACH TUNABLE KEEPING THE OTHERS FIXED TO DEFAULTS
# WHEN ADDING NEW PARAMETERS TO TEST
# NEED TO ADD TO TUNABLES THE LIST OF VALUES OF THAT PARAMETER
# TO DEFS THE NEW PARAMETER DEFAULT
# AND ADD IT IN THE "PROFILING" FUNCTION (EITHER LOOP AND PARAMETER PASSED
# TO MAKEFILE FOR COMPILATION)

tunables = {
    "SIZE_T": ["PRIMES", "TWOPOWERS"],
    "INIT_SIZE": ["256", "512", "1024"],
    "SIZE_MAX_G": ["19", "20"],
    "PROBING": ["LINEAR", "QUADRATIC"],
    "ALPHA": ["125", "375", "500", "625", "750", "875"],
    "HASH": ["FNV1a", "MURMUR", "DJB2"] 
}

# Default values
defs = {
    "SIZE_T": ["TWOPOWERS"],
    "INIT_SIZE": ["512"],
    "SIZE_MAX_G": ["20"],
    "PROBING": ["LINEAR"],
    "ALPHA": ["500"],
    "HASH": ["FNV1a"]
}

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Run profiling script with various configurations.")
parser.add_argument("-n", type=int, default=1, help="Number of repetitions")
args = parser.parse_args()
N = args.n

# Directories
root_dir = "prof"
measures_dir = os.path.join(root_dir, "measures")
images_dir = os.path.join(root_dir, "images")


def clean_dir(dir):
    os.makedirs(dir, exist_ok=True)
    for filename in os.listdir(dir):
        file_path = os.path.join(dir, filename)
        if os.path.isfile(file_path):
            os.unlink(file_path)

def save_picture(tun, df_key, key_str):
    plt.figure(figsize=(10, 6))
    
    for key, df in df_key.items():
        plt.plot(df["Number of Words"], df["Time"], label=key)

    plt.xscale("log", base=2)  # Set X-axis to log base 2
    plt.xlabel("Number of Words")
    plt.ylabel("Time (s)")

    # Force Y-axis to scientific notation with 1e-6 scale
    formatter = ScalarFormatter(useMathText=True)
    formatter.set_powerlimits((-6, -6))
    plt.gca().yaxis.set_major_formatter(formatter)

    plt.title(key_str + " mean lookup time comparison")
    plt.grid(True)
    plt.legend(title=key_str)
    plt.tight_layout()
    plt.savefig(os.path.join(images_dir, key_str))


def compute_and_save(tun, filename_ph, key_str):             
    df_key = {}
    for val in tun:
        filename_norep = filename_ph.replace("XXX", val)
        dataframes_reps = []
        
        for k in range(1,N+1):
            filename = "_".join([filename_norep, str(k)])
            file_path = os.path.join(measures_dir, filename)
            df = pd.read_csv(file_path)
            dataframes_reps.append(df)
        
        merged_df = dataframes_reps[0]
        
        for i, df in enumerate(dataframes_reps[1:], start=1):
            suffixes = (f'_left{i}', f'_right{i}')
            merged_df = pd.merge(merged_df, df, on='Number of Words', how='outer', suffixes=suffixes)

        if(N > 1):
            time_cols = [col for col in merged_df.columns if re.match(r'Time(_[a-z]+)?$', col)]
            merged_df['Time'] = merged_df[time_cols].mean(axis=1)
            merged_df = merged_df.drop([col for col in time_cols if col != 'Time'], axis=1)
    
        df_key.setdefault(val, [])
        df_key[val] = merged_df
    save_picture(tun, df_key, key_str)


def profiling(tunables, defs):
    for key in tunables.keys():
        # Create tmp dict with every key to default except the one to test
        tmp_t = tunables.copy()
        for tmp_key in tmp_t.keys():
            if(tmp_key == key):
                continue
            tmp_t[tmp_key] = defs[tmp_key]

        # Loop through all combinations
        for SIZE_T, INIT_SIZE, SIZE_MAX_G, PROBING, ALPHA, HASH in product(tmp_t["SIZE_T"], tmp_t["INIT_SIZE"], tmp_t["SIZE_MAX_G"], tmp_t["PROBING"], tmp_t["ALPHA"], tmp_t["HASH"]):
            print(f">>> Config: HT_SIZE_TYPE={SIZE_T}, HT_INITIAL_SIZE={INIT_SIZE}, "
                  f"HT_SIZE_MAX_GROWINGS={SIZE_MAX_G}, PROBING={PROBING}, ALPHA={ALPHA}, HASH_FUNCTION={HASH}")

            subprocess.run(["make", "clean"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
            for i in range(1, N + 1):
                VALUE = f"{SIZE_T}_{INIT_SIZE}_{SIZE_MAX_G}_{PROBING}_{ALPHA}_{HASH}_{i}"
                PROF_OFILE = f"prof/measures/{VALUE}"
                print(f"  Run {i}/{N}")
                
                try:
                    subprocess.run([
                        "make", "profiling",
                        f"ALPHA={ALPHA}",
                        f"PROBING={PROBING}",
                        f"HT_INITIAL_SIZE={INIT_SIZE}",
                        f"HT_SIZE_TYPE={SIZE_T}",
                        f"HT_SIZE_MAX_GROWINGS={SIZE_MAX_G}",
                        f"HASH_FUNCTION={HASH}",
                        f"PROF_OFILE={PROF_OFILE}"
                    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
                except subprocess.CalledProcessError:
                    print(f"[ERROR] Make failed on iteration {i}")




clean_dir(images_dir)
clean_dir(measures_dir)

profiling(tunables, defs)


filename_ph = "_".join(defs["SIZE_T"] + defs["INIT_SIZE"] + defs["SIZE_MAX_G"]
                       + defs["PROBING"] + defs["ALPHA"] + ["XXX"])

compute_and_save(tunables["HASH"], filename_ph, "hash")

filename_ph = "_".join(["XXX"] + defs["INIT_SIZE"] + defs["SIZE_MAX_G"]
                       + defs["PROBING"] + defs["ALPHA"] + defs["HASH"])

compute_and_save(tunables["SIZE_T"], filename_ph, "size_type")

filename_ph = "_".join(defs["SIZE_T"] + defs["INIT_SIZE"] + ["XXX"]
                       + defs["PROBING"] + defs["ALPHA"] + defs["HASH"])

compute_and_save(tunables["SIZE_MAX_G"], filename_ph, "size_max_grow")

filename_ph = "_".join(defs["SIZE_T"] + defs["INIT_SIZE"] + defs["SIZE_MAX_G"]
                       + ["XXX"] + defs["ALPHA"] + defs["HASH"])

compute_and_save(tunables["PROBING"], filename_ph, "probing")

filename_ph = "_".join(defs["SIZE_T"] + defs["INIT_SIZE"] + defs["SIZE_MAX_G"] 
                       + defs["PROBING"] + ["XXX"] + defs["HASH"])

compute_and_save(tunables["ALPHA"], filename_ph, "alpha")
