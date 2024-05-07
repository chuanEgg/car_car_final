from flask import Flask, render_template, jsonify, request, g
import sqlite3

database_name = '../SQLite Database/database.db'

app = Flask(__name__)

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
		cursor.execute("UPDATE Controls SET location_ctrl = ?",(location_code,))
		conn.commit()
	cursor.execute("SELECT Chinese_Name FROM Location_Names")
	name_list = cursor.fetchall()
	cursor.execute("SELECT code FROM Location_Names")
	code_list = cursor.fetchall()
	
	cursor.execute("SELECT location_ctrl FROM Controls")
	default_location_code = cursor.fetchall()[0][0]

	for i in range(len(name_list)):
		name_list[i] = name_list[i][0]
		code_list[i] = code_list[i][0]
 
	return render_template('index.html', names=name_list, codes = code_list, length = len(code_list), default_location_code = default_location_code)

@app.route('/left_button_press', methods=['POST'])
def left_button_press():

	try:
		conn = get_db()
		cursor = conn.cursor()
		cursor.execute("UPDATE Controls SET ctrl = -1")
		conn.commit()
	except:
		pass
	
	return jsonify({"success": True})

@app.route('/right_button_press', methods=['POST'])
def right_button_press():
	try:
		conn = get_db()
		cursor = conn.cursor()
		cursor.execute("UPDATE Controls SET ctrl = 1")
		conn.commit()
	except:
		pass

	return jsonify({"success": True})

if __name__=='__main__': 
	app.run(host="0.0.0.0", port=5000, debug=True) 