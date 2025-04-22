import argparse
import pandas as pd
import matplotlib.pyplot as plt
import os
import shutil
import subprocess
import itertools
from itertools import product

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Run profiling script with various configurations.")
parser.add_argument("-n", type=int, default=1, help="Number of repetitions")
args = parser.parse_args()
N = args.n


# Define possible values
root_dir = "./prof"
measures_dir = root_dir + "/measures"
images_dir = root_dir + "/images"
HT_SIZE_TYPES = ["PRIMES", "TWOPOWERS"]
HT_INITIAL_SIZES = ["512"]
HT_SIZE_MAX_GROWINGS = ["20"]
PROBING_METHODS = ["LINEAR", "QUADRATIC"]
ALPHAS = ["125", "375", "500", "625", "750", "875"]

default_size_type = "PRIMES"
default_prob = "LINEAR"
default_size_g = "20"
default_size_i = "512"
default_prob = "LINEAR"
default_alpha = "500"

df_size_type={}
df_initial_size={}
df_size_max_g={}
df_probing={}
df_alpha={}

# Clean the prof/measures directory
os.makedirs("./prof/measures", exist_ok=True)
for filename in os.listdir("./prof/measures"):
    file_path = os.path.join("./prof/measures", filename)
    if os.path.isfile(file_path):
        os.unlink(file_path)


# Loop through all combinations
for HT_SIZE_TYPE, HT_INITIAL_SIZE, HT_SIZE_MAX_GROWING, PROBING, ALPHA in product(
    HT_SIZE_TYPES, HT_INITIAL_SIZES, HT_SIZE_MAX_GROWINGS, PROBING_METHODS, ALPHAS
):
    print(f">>> Config: HT_SIZE_TYPE={HT_SIZE_TYPE}, HT_INITIAL_SIZE={HT_INITIAL_SIZE}, "
          f"HT_SIZE_MAX_GROWINGS={HT_SIZE_MAX_GROWING}, PROBING={PROBING}, ALPHA={ALPHA}")
    for i in range(1, N + 1):
        VALUE = f"{HT_SIZE_TYPE}_{HT_INITIAL_SIZE}_{HT_SIZE_MAX_GROWING}_{PROBING}_{ALPHA}_{i}"
        PROF_OFILE = f"prof/measures/{VALUE}"
        print(f"  Run {i}/{N}")
        
        try:
            subprocess.run(["make", "clean"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
            subprocess.run([
                "make", "profiling",
                f"ALPHA={ALPHA}",
                f"PROBING={PROBING}",
                f"HT_INITIAL_SIZE={HT_INITIAL_SIZE}",
                f"HT_SIZE_TYPE={HT_SIZE_TYPE}",
                f"HT_SIZE_MAX_GROWINGS={HT_SIZE_MAX_GROWING}",
                f"PROF_OFILE={PROF_OFILE}"
            ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
        except subprocess.CalledProcessError:
            print(f"[ERROR] Make failed on iteration {i}")


def compute_and_print(key):
    match key:
        case "alpha":
            vec = ALPHAS
            df_key = df_alpha
            filename_ph = default_size_type+"_"+default_size_i+"_"+default_size_g+"_"+default_prob+"_"+"XXX"
        case "probing":
            vec = PROBING_METHODS
            df_key = df_probing
            filename_ph = default_size_type+"_"+default_size_i+"_"+default_size_g+"_"+"XXX"+"_"+default_alpha
        case "size_type":
            vec = HT_SIZE_TYPES
            df_key = df_size_type
            filename_ph = "XXX"+"_"+default_size_i+"_"+default_size_g+"_"+default_prob+"_"+default_alpha
        case "size_growings":
            vec = HT_SIZE_MAX_GROWINGS
            df_key = df_size_max_g
            filename_ph = default_size_type+"_"+default_size_i+"_"+"XXX"+"_"+default_prob+"_"+default_alpha
        case "initial_size":
            vec = HT_INITIAL_SIZES
            df_key = df_initial_size
            filename_ph = default_size_type+"_"+"XXX"+"_"+default_size_g+"_"+default_prob+"_"+default_alpha
            
    for val in vec:
        filename_norep = filename_ph.replace("XXX", val)
        dataframes_reps = []
        
        for k in range(1,N+1):
            filename = filename_norep+"_"+str(k)
            file_path = os.path.join(measures_dir, filename)
            df = pd.read_csv(file_path)
            dataframes_reps.append(df)
        
        merged_df = dataframes_reps[0]
        
        for df in dataframes_reps[1:]:
            merged_df = pd.merge(merged_df, df, on='Number of Words', how='outer')

        print(merged_df)
    
        if(N > 1):
            merged_df['Time'] = merged_df[['Time_x', 'Time_y', 'Time']].mean(axis=1)
            merged_df = merged_df.drop(['Time_x', 'Time_y'], axis=1)
    
    
        df_key.setdefault(val, [])
        df_key[val].append(merged_df)
    
    plt.figure(figsize=(10, 6))
    for k in vec:
        dfs = df_key.get(k)
        for df in dfs:
            plt.plot(df["Number of Words"], df["Time"], label=k)
    
    plt.xlabel("Number of Words")
    plt.ylabel("Time")
    plt.title(key + " comparison")
    plt.grid(True)
    plt.legend(title=key)
    plt.tight_layout()
    plt.savefig(os.path.join(images_dir, key))


compute_and_print("alpha")
compute_and_print("probing")
compute_and_print("size_type")
compute_and_print("initial_size")
