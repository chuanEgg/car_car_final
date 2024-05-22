import csv
import sqlite3

def execute_sql_from_file(filename, db_name):
    # Connect to SQLite database (or create it if it doesn't exist)
    conn = sqlite3.connect(db_name)
    cursor = conn.cursor()
    
    # Read and execute the SQL commands from the .sql file
    with open(filename, 'r',encoding="utf8") as sql_file:
        sql_script = sql_file.read()
    cursor.executescript(sql_script)
    
    print(f"Successfully executed the SQL script from {filename} on database {db_name}")
    
    # Commit changes and close the connection
    conn.commit()
    conn.close()
    
def add_data_to_database(db_name):
    # Connect to SQLite database
    conn = sqlite3.connect(db_name)
    cursor = conn.cursor()
    
    # Read data from a CSV file and insert it into the database
    with open('location_name.csv', 'r',encoding="utf8") as csv_file:
        csv_reader = csv.reader(csv_file)
        for row in csv_reader:
            cursor.execute("INSERT INTO Location_Names (id, Postal_Code, Chinese_Name, English_Name, Weather_Forecast_Code_36_Hours_ID, Weather_Forecast_Code_2_Days_ID, Weather_Forecast_Code_7_Days_ID) VALUES (?, ?, ?, ?, ?, ?, ?)", (int(''.join(filter(str.isdigit, row[0]))), row[1], row[2], row[3], row[4], row[5], row[6]))
            cursor.execute("INSERT INTO City_Weather_36hr (id, Postal_Code, Chinese_Name, English_Name ) VALUES (?, ?, ?, ?)", (int(''.join(filter(str.isdigit, row[0]))), row[1], row[2], row[3]))
            cursor.execute("INSERT INTO City_Township_Weather_3_Days (id, Postal_Code, Chinese_Name, English_Name ) VALUES (?, ?, ?, ?)", (int(''.join(filter(str.isdigit, row[0]))), row[1], row[2],row[3]))
            
            
    cursor.execute("INSERT INTO Controls (page_ctrl,page_activation,location_id_ctrl) VALUES (?,?,?)", (0,-2147483648,0) )

    print(f"Successfully added data to the database {db_name}")
    
    # Commit changes and close the connection
    conn.commit()
    conn.close()

# Specify the path to your .sql file and the name of the database
sql_file_path = 'schema.sql'
database_name = 'database.db'

# Execute the function with your .sql file and database name
execute_sql_from_file(sql_file_path, database_name)
add_data_to_database(database_name)

