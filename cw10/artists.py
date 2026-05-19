import sys
import requests
import time

URL = "https://api.discogs.com/artists"


def get_json(url: str) -> dict | None:
    while True:
        try: 
            response = requests.get(url, timeout=10, headers={"User-Agent": "ApplicationToLearn/0.1"})

        except requests.exceptions.Timeout:
            print(f"fail: connection timed out for {url}")
            return None
            
        except requests.RequestException as e:
            print(f"fail: {e}")
            return None
        
        if response.status_code == 404:
            return None

        if response.status_code == 429:
            print("Rate limit exceeded (HTTP 429). Waiting 5 seconds before retrying...")
            time.sleep(5)
            continue

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

def find_common_groups(artists: list[dict]) -> None:
    
    groups_by_id = {}

    for artist in artists:
        for group in artist.get("groups", []):
            group_id = group["id"]
            
            if group_id not in groups_by_id:
                groups_by_id[group_id] = {
                    "name": group["name"],
                    "members": []
                }
            
            groups_by_id[group_id]["members"].append(artist["name"])

    common_groups = []
    for group_data in groups_by_id.values():
        if len(group_data["members"]) > 1:
            common_groups.append(group_data)

    if len(common_groups) == 0:
        return

    def get_group_name(group_dict):
        return group_dict["name"]

    common_groups.sort(key=get_group_name)

    for group in common_groups:
        members_str = ", ".join(group["members"])
        print(f"{group['name']}: {members_str}")


if __name__ == "__main__":

    if len(sys.argv) < 3:
         print("Usage: python3 artists.py <artist_id1> <artist_id2> ... ")
         sys.exit(1)

    artist_ids = sys.argv[1:]
    artists = []

    for artist_id in artist_ids:
        info = get_artist_info(artist_id)
        if info is None:
            print(f"Error: Cannot fetch data for ID '{artist_id}' (does not exist or API error).")
            sys.exit(1)
        artists.append(info)

    find_common_groups(artists)
