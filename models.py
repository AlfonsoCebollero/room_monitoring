from flask_sqlalchemy import SQLAlchemy
from flask import Flask
import os

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URL')
db = SQLAlchemy(app)


class User(db.Model):
    __tablename__ = "users"
    username = db.Column(db.String, unique=True, primary_key=True)
    password = db.Column(db.String, nullable=False)


db.create_all()
