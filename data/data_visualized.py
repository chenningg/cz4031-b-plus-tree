import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# open and read the data file
dir_path = os.path.dirname(os.path.realpath(__file__))
abs_file_path = os.path.join(dir_path, "data.tsv")
df = pd.read_csv(abs_file_path, sep='\t')
print(df.head(5))

# aggregate over numVotes
duplicate_numVotes = df.pivot_table(index=['numVotes'], aggfunc='size')
print(duplicate_numVotes)

# logscale since expecting imbalanced data
duplicate_numVotes = np.log2(duplicate_numVotes)

# visualize and save
ax = duplicate_numVotes.plot.hist(alpha=0.5)
ax.set_xlabel("log2(numVotes)")
ax.set_ylabel("log2(frequency)")

abs_save_path = os.path.join(dir_path, "numVotes_frequency.png")
plt.savefig(abs_save_path)

plt.show()