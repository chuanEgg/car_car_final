from flask import Flask, render_template, jsonify, request, g, redirect, url_for
import asyncio
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
 
	cursor.execute("SELECT location_id_ctrl , motor_pwm_ctrl, motor_time_duration_ctrl, fan_pwm_ctrl, fan_time_duration_ctrl FROM Controls")
	result = cursor.fetchall()
	print(result)
	default_location_code = result[0][0]
	default_motor_pwm = result[0][1]
	default_motor_time_duration = result[0][2]
	default_fan_pwm = result[0][3]
	default_fan_time_duration = result[0][4]
 
	if request.method == 'POST':
		location_code_new = request.form['dropdown']
		motor_pwm_new = request.form['motor_pwm']
		motor_time_duration_new = request.form['motor_time_duration']
		fan_pwm_new = request.form['fan_pwm']
		fan_time_duration_new = request.form['fan_time_duration']
		# print(request.form)
  		#send the new location code to  main thread via fifo
		query = "UPDATE Controls SET location_id_ctrl=?, motor_pwm_ctrl=?, motor_time_duration_ctrl=?, fan_pwm_ctrl=?, fan_time_duration_ctrl=?"
		# Execute the query with the parameters
		cursor.execute(query, (location_code_new, motor_pwm_new, motor_time_duration_new, fan_pwm_new, fan_time_duration_new))
		conn.commit()
		if( int(location_code_new) != int(default_location_code) ):
			message = "4:"+str(location_code_new)
			send_data_to_LED_Matrix(message)
			print(location_code_new)
   
		elif( int(motor_pwm_new) != int(default_motor_pwm) ):
			message = "5:"+str(motor_pwm_new)
			send_data_to_LED_Matrix(message)
			print(motor_pwm_new)

		elif( int(motor_time_duration_new) != int(default_motor_time_duration) ):
			message = "6:"+str(motor_time_duration_new)
			send_data_to_LED_Matrix(message)
			print(motor_time_duration_new)

		elif( int(fan_pwm_new) != int(default_fan_pwm) ):
			message = "7:"+str(fan_pwm_new)
			send_data_to_LED_Matrix(message)
			print(fan_pwm_new)
   
		elif( int(fan_time_duration_new) != int(default_fan_time_duration) ):	
			message = "8:"+str(fan_time_duration_new)
			send_data_to_LED_Matrix(message)
   
 
	cursor.execute("SELECT Chinese_Name FROM Location_Names")
	name_list = cursor.fetchall()
	cursor.execute("SELECT id FROM Location_Names")
	code_list = cursor.fetchall()
	
	for i in range(len(name_list)):
		name_list[i] = name_list[i][0]
		code_list[i] = code_list[i][0]
  
	cursor.execute("SELECT location_id_ctrl , motor_pwm_ctrl, motor_time_duration_ctrl, fan_pwm_ctrl, fan_time_duration_ctrl FROM Controls")
	result = cursor.fetchall()
	print(result)
	default_location_code = result[0][0]
	default_motor_pwm = result[0][1]
	default_motor_time_duration = result[0][2]
	default_fan_pwm = result[0][3]
	default_fan_time_duration = result[0][4]

	# cursor.close() 

	return render_template('index.html', names=name_list, codes = code_list, length = len(code_list), location_code = default_location_code, motor_pwm = default_motor_pwm, motor_time_duration = default_motor_time_duration, fan_pwm = default_fan_pwm, fan_time_duration = default_fan_time_duration)

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

@app.route('/submit', methods=['POST'])
def submit():
    motor_pwm = request.form['motor_pwm']
    motor_time_duration = request.form['motor_time_duration']
    fan_pwm = request.form['fan_pwm']
    fan_time_duration = request.form['fan_time_duration']
    
    conn = get_db()
    cursor = conn.cursor() 
    
    
    # Commit the changes
    conn.commit()

	# Close the cursor and connection
    cursor.close()
    conn.close()
    return redirect(url_for('hello'), code=200)


if __name__=='__main__': 
  
	app.run(host="0.0.0.0", port=5000, debug=True) 