import sys
import requests
import json

URL = "https://api.discogs.com/artists"


def get_artist_info(artist_id: str) -> dict | None:
    url = f"https://api.discogs.com/artists/{artist_id}"
    try:
        response = requests.get(url, timeout=10, headers={"User-Agent": "ApplicationToLearn/0.1"})
    except requests.RequestException as e:
        print(f"fail: {e}")
        return None

    if response.status_code != 200:
        print(f"fail: status {response.status_code}")
        return None

    try:
        data = response.json()
    except ValueError:
        print("fail: response is not valid JSON")
        return None

    return {
        "id": artist_id,
        "name": data.get("name", "Unknown"),
        "groups": data.get("groups", []),
    }

def search_albums(url: str, artist_id: str) -> list:
    try:
        new_url = f"{url}/{artist_id}/releases"

        response = requests.get(new_url, timeout=10, headers={"User-Agent":"ApplicationToLearn/0.1"})
    
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

    # print(json.dumps(data["releases"], indent=2))

    for release in data["releases"]:
        print(f"Title: {release['title']}")

    # print(data)

if __name__ == "__main__":

    artist_ids = []

    while not artist_ids:

        artist_ids = input("Please, provide IDs of the artists you want to compare: ").split()

        if not artist_ids:
            print("Please, provide at least one artist")


    # for artist_id in artist_ids:
    #     search_albums(URL, artist_id)

    for artist_id in artist_ids:
        info = get_artist_info(artist_id)
        if info:
            print(f"{info['name']}: {[g['name'] for g in info['groups']]}")