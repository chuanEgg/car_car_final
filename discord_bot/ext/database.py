import discord
from discord import app_commands
from discord.ext import commands
import sqlite3
import os, socket
# from flask import Flask, render_template, jsonify, request, g

database_name = os.path.join(os.path.dirname(__file__), '..', '..', 'SQLite Database', 'database.db')
# Define the server address and port
LED_Matrix_server_address = ('localhost', 15000)

class database(commands.Cog):
    def __init__(self, bot):
        self.bot = bot
        self.database = sqlite3.connect(database_name)
        self.cursor = self.database.cursor()
    
    # deal with the commands.
    @app_commands.command(name="prev", description="switch to previous page")
    async def prev(self, interaction: discord.Interaction):
        # send command to LED matrix program
        cmd = r"1:0"
        self.send_data_to_LED_Matrix(cmd)
        
        # write to database
        self.cursor.execute("UPDATE Controls SET page_ctrl = -1")
        self.database.commit()
        # send message to user
        await interaction.response.send_message("Switched to previous page", ephemeral=True)

    @app_commands.command(name="next", description="switch to next page")
    async def next(self, interaction: discord.Interaction):
        # send command to LED matrix program
        cmd = r"0:0"
        self.send_data_to_LED_Matrix(cmd)
        
        # write to database
        self.cursor.execute("UPDATE Controls SET page_ctrl = 1")
        self.database.commit()
        # send message to user
        await interaction.response.send_message("Switched to next page", ephemeral=True)
    
    # to list all locations    
    # still under construction
    @app_commands.command(name="list", description="list all locations")
    async def list_location(self, interaction: discord.Interaction):
        	
        self.cursor.execute("SELECT id , Chinese_Name FROM Location_Names")
        response = self.cursor.fetchall()
        output = "id / Chinese Name\n"
        for i in range(len(response)):
            output += f"{response[i][0]} : {response[i][1]}\n"
        
        # send message to user
        await interaction.response.send_message(output, ephemeral=True)
    
    # send data to main thread by socket
    def send_data_to_LED_Matrix(self, data:str):
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




async def setup(bot):
    await bot.add_cog(database(bot))