import sys
import requests

URL = "https://api.discogs.com/artists/"

def search_albums(url: str, #artist_id: list
                  ) -> int:
    try:
        response = requests.get(url, timeout=10, headers={"User-Agent":"ApplicationToLearn/0.1"})
    
    except requests.exceptions.Timeout:
        print(f"fail: cannot connect to {url}")
        return 1

    except requests.RequestException as e:
        print(f"fail: Request error: {e}")
        return 1
    
    if response.status_code != 200:
        print(f"fail: server responded with status code: {response.status_code}")
        return 1

    try:
        data = response.json()
    
    except ValueError:
        print("fail: response is not valid JSON")
        return 1


#    print(data.keys())
#    for key,value in data.items().artist:
#        print(f"{key}: {value}")

    print(data["artists"])

if __name__ == "__main__":

    artist_ids = []

    while not artist_ids:

        artist_ids = input("Please, provide IDs of the artists you want to compare: ").split()

        if not artist_ids:
            print("Please, provide at least one artist")



    search_albums(URL)