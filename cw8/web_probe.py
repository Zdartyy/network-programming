import sys
import requests

URL = "http://th.if.uj.edu.pl/"
EXPECTED_TEXT = "Institute of Theoretical Physics"

def check_site(url: str, expected: str) -> int:
    try:
        response = requests.get(url, timeout=10)
        
    except requests.exceptions.ConnectionError:
        print(f"fail: cannot connect to {url}")
        return 1
    except requests.exceptions.Timeout:
        print(f"fail: Timeout exceeded for {url}")
        return 1
    except requests.exceptions.RequestException as e:
        print(f"fail: Request error: {e}")
        return 1

    if response.status_code != 200:
        print(f"fail: Server returned status {response.status_code} (expected 200)")
        return 1

    content_type = response.headers.get("Content-Type", "")

    if "text/html" not in content_type:
        print(f"fail: Invalid Content-Type: '{content_type}' (expected text/html)")
        return 1

    if expected not in response.text:
        print(f"fail: Text '{expected}' not found in page content")
        return 1

    print(f"Site {url} is working correctly")
    print(f"Status: {response.status_code}")
    print(f"Content-Type: {content_type}")
    print(f"Text '{expected}': found")
    return 0

if __name__ == "__main__":
    sys.exit(check_site(URL, EXPECTED_TEXT))