import os
import sys
import csv
import re
import binascii
import array
import shutil

def parseCSV(input):
	result = []
	filename = 'header'
	outputFilename = 'output.pbm'
	
	with open(input, 'rb') as csvfile:
		rows = csv.reader(csvfile, delimiter = ',')
		count = 0
		hex4 = ""
		for row in rows:
			row = [int(x) for x in row]

			if sum(row) < 60:
				hex4 = hex4 + (format(0, '01x'))
			elif sum(row) < 90:
				hex4 = hex4 + (format(1, '01x'))
			elif sum(row) < 125:
				hex4 = hex4 + (format(2, '01x'))
			elif sum(row) < 155:
				hex4 = hex4 + (format(3, '01x'))
			elif sum(row) < 192:
				hex4 = hex4 + (format(4, '01x'))
			elif sum(row) < 220:
				hex4 = hex4 + (format(5, '01x'))
			elif sum(row) < 252:
				hex4 = hex4 + (format(6, '01x'))
			elif sum(row) < 287:
				hex4 = hex4 + (format(7, '01x'))
			elif sum(row) < 313:
				hex4 = hex4 + (format(8, '01x'))
			elif sum(row) < 338:
				hex4 = hex4 + (format(9, '01x'))
			elif sum(row) < 368:
				hex4 = hex4 + (format(10, '01x'))
			elif sum(row) < 397:
				hex4 = hex4 + (format(11, '01x'))
			elif sum(row) < 430:
				hex4 = hex4 + (format(12, '01x'))
			elif sum(row) < 458:
				hex4 = hex4 + (format(13, '01x'))
			elif sum(row) < 481:
				hex4 = hex4 + (format(14, '01x'))
			else:
				hex4 = hex4 + (format(15, '01x'))

		temp = ""
		shutil.copy('header', 'header_backup')
		for c in hex4:
			if len(temp) == 2:
				f = open(filename, 'a')
				f.write(binascii.unhexlify(temp))
				f.close()
				count += 1
				temp = ""

			temp += c

		f = open(filename, 'a')
		f.write(binascii.unhexlify(temp))
		f.close()
		count += 1

                os.rename(filename, outputFilename)
                os.rename('header_backup', 'header')


if __name__ == "__main__":
	if len(sys.argv) != 2:
		print "USAGE: python collect_image_line_counting.py 01.csv"
		sys.exit(0)

	bi5 = []
	filename = 'header'
	parseCSV(sys.argv[1])
