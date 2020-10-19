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

max_votes = max(duplicate_numVotes)

# logscale since expecting imbalanced data
duplicate_numVotes = np.log10(duplicate_numVotes)

# visualize and save
ax = duplicate_numVotes.plot.hist(bins=20)
ax.set_xlabel("log10(numVotes)")
ax.set_ylabel("log10(frequency)")
ax.set_title("Frequency of votes against number of votes. The max votes: " + str(max_votes))
abs_save_path = os.path.join(dir_path, "numVotes_frequency.png")
plt.savefig(abs_save_path)


plt.show()
