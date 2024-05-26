import discord
from discord import app_commands
from discord.ext import commands
import sqlite3
import os
# from flask import Flask, render_template, jsonify, request, g

database_name = os.path.join(os.path.dirname(__file__), '..', '..', 'SQLite Database', 'database.db')

class database(commands.Cog):
    def __init__(self, bot):
        self.bot = bot
        self.database = sqlite3.connect(database_name)
        self.cursor = self.database.cursor()
    
    # deal with the commands.
    @app_commands.command(name="prev", description="switch to previous page")
    async def prev(self, interaction: discord.Interaction):
        self.cursor.execute("UPDATE Controls SET ctrl = -1")
        self.database.commit()
        await interaction.response.send_message("Switched to previous page", ephemeral=True)

    @app_commands.command(name="next", description="switch to next page")
    async def next(self, interaction: discord.Interaction):
        self.cursor.execute("UPDATE Controls SET ctrl = 1")
        self.database.commit()
        await interaction.response.send_message("Switched to next page", ephemeral=True) 


async def setup(bot):
    await bot.add_cog(database(bot))