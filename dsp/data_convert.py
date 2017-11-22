
f = open("test_data.txt", "r")
fout = open("test_data_commas.txt", "w")

for line in f:
	fout.write(line.replace(" ", ","))

f.close()
fout.close()