from flask import Flask, render_template, jsonify, request, g
import sqlite3, socket

# Define the location of database file
database_name = '../SQLite Database/database.db'
# Define the server address and port
LED_Matrix_server_address = ('localhost', 15000)


app = Flask(__name__)


def send_data_to_LED_Matrix(data:str):
	try:
		# Create a socket object
		client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		# Connect to the server
		client_socket.connect(LED_Matrix_server_address)
		# Send data
		data = data.encode('ascii')
		client_socket.sendall(data)

		# Close the socket
		client_socket.close()
	except Exception as e:
		print("Error in sending data to LED Matrix: ", e)


def get_db(): 
    if 'db' not in g:
        # Open a new database connection if there is none yet for the current application context.
        g.db = sqlite3.connect(database_name)
            
    return g.db

@app.teardown_appcontext
def close_db(e=None):
    db = g.pop('db', None)
    if db is not None:
        db.close()
        
        
@app.route('/',methods=['GET','POST'])	 
def hello():
	conn = get_db()
	cursor = conn.cursor() 
	if request.method == 'POST':
		location_code = request.form['dropdown']
  		#send the new location code to  main thread via fifo
		message = "4:"+str(location_code)
		send_data_to_LED_Matrix(message)
  
		# update the location code in the database
		cursor.execute("UPDATE Controls SET location_id_ctrl = ?",(location_code,))
		conn.commit()
	cursor.execute("SELECT Chinese_Name FROM Location_Names")
	name_list = cursor.fetchall()
	cursor.execute("SELECT id FROM Location_Names")
	code_list = cursor.fetchall()
	
	cursor.execute("SELECT location_id_ctrl FROM Controls")
	default_location_code = cursor.fetchall()[0][0]

	for i in range(len(name_list)):
		name_list[i] = name_list[i][0]
		code_list[i] = code_list[i][0]
 
	return render_template('index.html', names=name_list, codes = code_list, length = len(code_list), default_location_code = default_location_code)

@app.route('/left_button_press', methods=['POST'])
def left_button_press():
	data = "1:0"
	send_data_to_LED_Matrix(data)
	# try:
	# 	conn = get_db()
	# 	cursor = conn.cursor()
	# 	cursor.execute("UPDATE Controls SET ctrl = -1")
	# 	conn.commit()
  

	# except:
	# 	pass
	
	return jsonify({"success": True})

@app.route('/right_button_press', methods=['POST'])
def right_button_press():
	data = "0:0"
	send_data_to_LED_Matrix(data)
	# try:
	# 	conn = get_db()
	# 	cursor = conn.cursor()
	# 	cursor.execute("UPDATE Controls SET ctrl = 1")
	# 	conn.commit()
	# except:
	# 	pass

	return jsonify({"success": True})

if __name__=='__main__': 
  
	app.run(host="0.0.0.0", port=5000, debug=True) 