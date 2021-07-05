import kivy
from kivy.app import App
from kivy.lang import Builder
from kivy.properties import ObjectProperty
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.popup import Popup
import requests


def close_popup():
    pop_up_window.disabled()


class Content(FloatLayout):
    def close_popup(self):
        pop_up_window.dismiss()


def show_popup():
    pop_up_window.open()


class MainWindow(Screen):
    user = ObjectProperty(None)
    passw = ObjectProperty(None)

    def login(self):
        if self.passw.text == "1234" and self.user.text == "Alfon":
            return True
        else:
            return False

    def bad_login(self):
        show_popup()


class SecondWindow(Screen):
    temp = ObjectProperty(None)
    lum = ObjectProperty(None)
    lock = ObjectProperty(None)

    def get_data(self):
        room_data = requests.get('http://127.0.0.1:5000/room_data/Salamander/60e2f585b3efe679775f095a')
        self.lum.text = room_data.json()['Luminosidad']
        self.temp.text = room_data.json()['Temperatura']

        if room_data.json()['Cerrojo'] == '1':
            self.lock.text = 'Abrir Cerrojo'
        else:
            self.lock.text = 'Cerrar Cerrojo'

    def update_lock(self):
        room_data = requests.get('http://127.0.0.1:5000/room_data/Salamander/60e2f585b3efe679775f095a')
        if room_data.json()['Cerrojo'] == '1':
            requests.post('http://127.0.0.1:5000/room_data/Salamander/60e2f585b3efe679775f095a?Cerrojo=0')
        else:
            requests.post('http://127.0.0.1:5000/room_data/Salamander/60e2f585b3efe679775f095a?Cerrojo=1')
        self.get_data()


class WindowManager(ScreenManager):
    pass


kv = Builder.load_file("my.kv")


class MyMainApp(App):
    def build(self):
        return kv


if __name__ == "__main__":
    content = Content()
    pop_up_window = Popup(title='Bad login', content=content, size_hint=(None, None), size=(400, 400))
    MyMainApp().run()
