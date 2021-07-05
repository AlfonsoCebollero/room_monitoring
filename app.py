from bson.json_util import dumps
import json
from flask import Flask, render_template, request, session, redirect, url_for
import os
from models import *
from flask_session import Session
from pymongo import MongoClient
from bson.objectid import ObjectId

mongo_client = MongoClient('localhost', 27017)
app = Flask(__name__)
app.config["SQLALCHEMY_DATABASE_URI"] = os.getenv("DATABASE_URL")
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False
db.init_app(app)
app.config["SESSION_PERMANENT"] = False
app.config["SESSION_TYPE"] = "filesystem"
Session(app)


def get_user_rooms(username):
    mongo_db = mongo_client.rooms
    user_collection = f'{username}_rooms'
    user_rooms = mongo_db[user_collection]
    return user_rooms


@app.route('/')
def hello_world():
    try:
        if session["authenticated"]:
            return redirect(url_for("load_profile", username=session.get("username")))
    except KeyError:
        print('Log in first, no authenticated key')
    return render_template("index.html", bad_login=False, message="", just_registered=False)


@app.route('/register')
def register():
    return render_template("signin.html")


@app.route('/profile', methods=["POST", "GET"])
def profile():
    if request.method == "POST":

        username = request.form.get("username")
        password = request.form.get("password")

        try:
            user = User.query.get(username)
            if user.password == password:
                session["authenticated"] = True
                session["username"] = username
                return redirect(url_for("load_profile", username=username))
            else:
                return render_template("index.html", message="Wrong Password", bad_login=True, just_registered=False)
        except:
            return render_template("index.html", message="No such Username", bad_login=True, just_registered=False)

    else:
        return render_template("index.html")


@app.route('/validation', methods=["POST"])
def insert_user():
    username = request.form.get("username")
    password = request.form.get("password")
    confirm = request.form.get("confirm")
    user = User(username=username, password=password)
    if User.query.get(username) is not None:
        return render_template("signin.html", username_used=True)
    else:
        pass

    if password == confirm:
        try:
            db.session.add(user)
            db.session.add(user)
            db.session.commit()
            return render_template("index.html", just_registered=True, bad_login=False,
                                   message="Succesfully registered, log in and enjoy the app!")
        except ValueError:
            print("Hello")
            return "Error, username already registered, try another"
    else:
        return render_template("signin.html")


@app.route('/profile/<string:username>', methods=["POST", "GET"])
def load_profile(username):
    user_rooms = get_user_rooms(username)
    try:
        rooms = list(user_rooms.find())
        print(rooms)
    except TypeError:
        print("No documents found")
        rooms = []
    if session.get("authenticated") is None:
        session["authenticated"] = False

    if session["authenticated"]:
        if len(rooms) == 0:
            return render_template("profile.html", username=username, rooms=None, alert=False)
        else:
            return render_template("profile.html", username=username, rooms=rooms, alert=False)
    else:
        return "BAD REQUEST"


@app.route('/logout', methods=["POST"])
def log_out():
    session["authenticated"] = False
    session["username"] = None
    return redirect(url_for("hello_world"))


@app.route('/create_room', methods=["POST"])
def create_room():
    username = session["username"]
    room_name = request.form.get("name")
    room = {'name': room_name}
    parameters = dict(request.form)
    parameters.pop('name')
    listed_parameters = list(parameters.values())
    for i, parameter in enumerate(listed_parameters):
        print(parameter)
        if (i % 2) == 0:
            parameter = parameter
            parameter_type = listed_parameters[i + 1]
            if parameter_type == '2':
                room[parameter] = 0
            else:
                room[parameter] = ""
        else:
            continue
    print(room)
    mongo_db = mongo_client.rooms
    user_collection = f'{username}_rooms'
    user_rooms = mongo_db[user_collection]
    room_to_create = user_rooms.find_one({'name': room_name})

    if room_to_create is None:
        user_rooms.insert_one(room)
        return redirect(url_for("load_profile", username=session.get("username")))
    else:
        return render_template("profile.html", username=username, rooms=None, alert=True)


@app.route('/room_data/<string:username>/<string:identifier>', methods=['POST', 'GET'])
def room_data(username, identifier):
    if request.method == 'GET':
        user_rooms = get_user_rooms(username)
        object_id = ObjectId(identifier)
        room = user_rooms.find({'_id': object_id}, {'_id': 0})
        if room is None:
            return "BAD REQUEST"
        else:
            data = dumps(room)
            treated_data = json.loads(data[1:-1])
            return treated_data
    else:
        user_rooms = get_user_rooms(username)
        object_id = ObjectId(identifier)
        print(request.args)
        updating_parameters = list(request.args)
        updating_values = list(request.args.values())
        print(updating_parameters)
        update_query = {}
        for i, parameter in enumerate(updating_parameters):
            update_query[parameter] = updating_values[i]
        print(update_query)
        mongo_query = {'_id': object_id}
        new_values = {'$set': update_query}
        user_rooms.update(mongo_query, new_values)
        return redirect(url_for("room_data", username=username, identifier=identifier))


if __name__ == '__main__':
    app.run()
