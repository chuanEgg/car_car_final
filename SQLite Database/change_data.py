#for editing csv file, just for testing
import csv

data = []
with open('location_name.csv', 'r',encoding="utf8") as csv_file:
    csv_reader = csv.reader(csv_file)
    for row in csv_reader:
        data.append(row)

with open('location_name.csv', 'w', newline='',encoding="utf8") as csv_file:
    csv_writer = csv.writer(csv_file)
    for row in data:
        row[3] = 'F-C0032-001'
        row[4] = 'F-D0047-'+ row[4]
        row[5] = 'F-D0047-'+row[5]
        csv_writer.writerow(row)