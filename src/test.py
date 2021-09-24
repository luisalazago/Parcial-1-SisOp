# This is a code to test the concurrence of the server.

import requests

def main():
    n = int(input("Put an N: "))
    URL = "http://localhost:9090/"
    k = 1
    for i in range(n):
        response = requests.get(URL)
        if(response.json()):
            if(k == 1): print("You made 1 request succesefully.")
            else: print("You made {} requests succesefully.".format(k))
            k += 1
        else: print("Error trying to process a request :/")
    print("\n============================================")
    if(k == n): print("The concurrence is right.")
    else: print("The concurrence has a possible mistake.")

main()