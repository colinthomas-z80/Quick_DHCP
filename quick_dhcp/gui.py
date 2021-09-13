import tkinter as tk
import os
import subprocess
from threading import Thread

window = tk.Tk()
window.configure(background="lightblue")

button_start = tk.Button(window, text="Start", command=lambda:start_button_action())
box_host_ip = tk.Text(window, height=1, width=15)
box_target_ip = tk.Text(window,height=1, width=15)
label_box_host_ip = tk.Label(window, text="Host/Inteface IP", background="lightblue")
label_box_target_ip = tk.Label(window, text="Target IP", background="lightblue")

def verify_ip_v4(ip):
    bites = ip.split(".")

    if(len(bites) != 4):
        print(ip)
        print(bites)
        print(len(bites))
        print("Verify length error")
        return False

    for i in bites:
        if int(i) > 255 or int(i) < 0:
            print("Verify Range Error")
            return False
    return True

def start_dhcp():
    host_ip = box_host_ip.get("1.0","end")
    target_ip = box_target_ip.get("1.0","end")

    host_ip = host_ip.strip("\n") # remove any user entered whitespace
    target_ip = target_ip.strip("\n")

    if(verify_ip_v4(host_ip) != True):
        print("Host IP missing or formatted incorrectly")
        if(verify_ip_v4(target_ip) != True):
            print("Target IP missing or formatted incorrectly")
        return
    
    if(verify_ip_v4(target_ip) != True):
        print("Target IP missing or formatted incorrectly")
        return
    
    print(".\\server.exe " + host_ip + " " + target_ip)
    subprocess.run(".\\server.exe " + host_ip + " " + target_ip)
        
def start_button_action():
    run_thread = Thread(target=start_dhcp)
    run_thread.start()

def init_window():
    window.title("Quick DHCP")
    window.geometry("240x240")

    button_start.place(x=20, y=30)
    box_host_ip.place(x=80, y=30)
    box_target_ip.place(x=80, y=75)

    label_box_host_ip.place(x=80, y=9)
    label_box_target_ip.place(x=80, y=54)

    window.mainloop()

init_window()