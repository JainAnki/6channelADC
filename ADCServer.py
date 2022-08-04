import asyncio
import websockets
import json
import numpy as np
import time
from json import JSONEncoder
import os
import csv
HOST = ""  # Empty denotes a localhost. Because RPi is sending to the server's localhost which is this laptop's localhost hence an empty string
PORT = 7891
CONNECTIONS = set()
chanData = np.eye(500,6)
parent_dir = "G:/Thesis/8_ch/files/"
global path
global filet  
filet = ''
global num
num = ''

class NumpyArrayEncoder(JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        return JSONEncoder.default(self, obj)

async def handler(websocket):
    CONNECTIONS.add(websocket)
    count = 0
    sendMsg = {}
    while True:
        try:
            #msg(await websocket.recv())
            message = json.loads(await websocket.recv())
            #print(message)
            if (message.get('source', 'sensor') == 'html'):
                folder_name = message.get('folderName')
                global num
                num = message.get('typeSig')

                print(num)
                if folder_name is not None and folder_name.strip() != '':
                    # Path

                    global path 
                    path = os.path.join(parent_dir, folder_name)
                    try: 
                        os.mkdir(path) 
                    except OSError as error: 
                        print(error) 

                    print("Directory '% s' created" % folder_name)
                    await websocket.send(json.dumps({"error": "Folder name received!"}))
                else:
                    await websocket.send(json.dumps({"error": "Folder name is required!"}))
                    print("send")
            else:
                if message["Force_FSR"] == "Don't plot":
                    print("break")
                    count = 501
                if count<500:
                    chanData[count] = message["Force_FSR"]
                count+=1
                sendMsg = {"Force_FSR":np.array(chanData).transpose(), "type" : num }
                if(path) and count<500:
                    row =list(map(str, message["Force_FSR"]))                   
                     # Creates a new file
                    # writing the data into the file
                    global filet
                    
                    if(num == "on"):   
                        #print("base")     
                        with  open(path+'/base.csv', 'a', newline ='') as fp:
                            write = csv.writer(fp)
                            write.writerow(list(row))
                            fp.close()
                           
                    if(num == "off"):     
                        #print("test")   
                        with  open(path+'/test.csv', 'a', newline ='') as fp:
                            write = csv.writer(fp)
                            write.writerow(list(row))
                        fp.close()
                sendMsg = json.dumps(sendMsg, cls=NumpyArrayEncoder)  # use dump() to write array into file

                # Send a response to all connected client except the server
                for conn in CONNECTIONS:
                    if conn != websocket and count == 500:  
                        #print("SendMsg")
                        print(count)         
                        print(conn)   
                        try:
                            await conn.send(sendMsg)
                            print("sentttttt")
                            if(conn == (list(CONNECTIONS)[-1])):
                                count = 0
                                print("send")
                        except websockets.exceptions.ConnectionClosedError as error1:                            
                            print(f'Server Error: {error1}')
                            if(conn == (list(CONNECTIONS)[-1])):
                                count = 0
                                print("send")
                            #count =0
                            #CONNECTIONS.remove(websocket)

                        #await asyncio.sleep(0.5)
        except websockets.exceptions.ConnectionClosedError as error1:
            print(f'Server Error: {error1}')
            count =0
            CONNECTIONS.remove(websocket)


async def main():
    async with websockets.serve(handler, HOST, PORT):
        await asyncio.Future()  # run forever


if __name__ == "__main__":

    asyncio.run(main())
