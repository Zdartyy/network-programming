import sys
import requests
import json

URL = "https://api.discogs.com/artists"


def get_json(url: str) -> dict | None:
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

    return data


def get_artist_info(artist_id: str) -> dict | None:

    url = f"{URL}/{artist_id}"
    data = get_json(url)

    if not data:
        return None

    return {
        "id": artist_id,
        "name": data.get("name", "Unknown"),
        "groups": data.get("groups", []),
    }

def find_common_groups(artists: list) -> None:

    groups_by_artist = {}
    for artist in artists:

        group_names = {g["name"] for g in artist["groups"]}

        groups_by_artist[artist["name"]] = group_names

    names = list(groups_by_artist.keys())

    found_common = False


    for i in range(len(names)):
        for j in range(i + 1, len(names)):
            name_a = names[i]
            name_b = names[j]
            common = groups_by_artist[name_a] & groups_by_artist[name_b]

            if common:
                found_common = True
                print(f"{name_a} and {name_b} played together in: {', '.join(common)}")

    if not found_common:
        print("No common groups found among the provided artists.")


if __name__ == "__main__":

    artist_ids = []

    while not artist_ids:

        artist_ids = input("Please, provide IDs of the artists you want to compare: ").split()

        if not artist_ids:
            print("Please, provide at least one artist")


    artists = []

    for artist_id in artist_ids:
        info = get_artist_info(artist_id)
        if info:
            artists.append(info)


    find_common_groups(artists)

