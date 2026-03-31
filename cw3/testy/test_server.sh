#!/bin/bash

SERVER="127.0.0.1"
PORT="2020"
PASS=0
FAIL=0

# $1 = opis testu, $2 = wejście (bez \n), $3 = oczekiwana odpowiedź
run_test() {
    local desc="$1"
    local input="$2"
    local expected="$3"

    actual=$(printf "%s" "$input" | socat -t 2.0 stdio udp4:${SERVER}:${PORT} 2>/dev/null)

    if [ "$actual" = "$expected" ]; then
        printf "[PASS] %s\n" "$desc"
        ((PASS++))
    else
        printf "[FAIL] %s\n" "$desc"
        printf "       oczekiwano: %s  bajty: %s\n" \
            "$expected" "$(printf '%s' "$expected" | od -A n -t u1 | tr -s ' ')"
        printf "       otrzymano:  %s  bajty: %s\n" \
            "$actual"   "$(printf '%s' "$actual"   | od -A n -t u1 | tr -s ' ')"
        ((FAIL++))
    fi
}

# Wersja z \n na końcu (serwer musi też akceptować)
run_test_nl() {
    local desc="$1"
    local input="$2"
    local expected="$3"

    actual=$(printf "%s\n" "$input" | socat -t 2.0 stdio udp4:${SERVER}:${PORT} 2>/dev/null)

    if [ "$actual" = "$expected" ]; then
        printf "[PASS] %s (z \\n)\n" "$desc"
        ((PASS++))
    else
        printf "[FAIL] %s (z \\n)\n" "$desc"
        printf "       oczekiwano: %s\n" "$expected"
        printf "       otrzymano:  %s\n" "$actual"
        ((FAIL++))
    fi
}

echo "========================================"
echo " Testy serwera palindromów UDP port $PORT"
echo "========================================"

echo ""
echo "--- Poprawne zapytania ---"
run_test    "jeden palindrom"               "kayak"                  "1/1"
run_test    "jeden nie-palindrom"           "hello"                  "0/1"
run_test    "dwa palindromy z dwóch"        "kayak racecar"          "2/2"
run_test    "dwa palindromy z czterech"     "kayak hello racecar ok" "2/4"
run_test    "zero palindromów"              "hello world"            "0/2"
run_test    "wyraz jednoliterowy"           "a"                      "1/1"
run_test    "wielka litera - Ala"           "Ala"                    "1/1"
run_test    "wielkie litery - KAYAK"        "KAYAK"                  "1/1"
run_test    "mieszane - Racecar"            "Racecar"                "1/1"
run_test    "dwa wyrazy jednoliterowe"      "a b"                    "2/2"
run_test    "jeden wyraz nie-palindrom"     "abc"                    "0/1"

echo ""
echo "--- Pusty datagram ---"
# socat nie obsługuje pustych datagramów — test manualny
echo "[SKIP] pusty datagram → 0/0  (socat nie wysyła pustych datagramów, przetestuj ręcznie)"

echo ""
echo "--- Akceptacja \\n i \\r\\n na końcu ---"
run_test_nl "z \\n na końcu - kayak"        "kayak"        "1/1"
run_test_nl "z \\n na końcu - hello world"  "hello world"  "0/2"

echo ""
echo "--- Błędne zapytania → ERROR ---"
run_test "spacja na początku"               " kayak"         "ERROR"
run_test "spacja na końcu"                  "kayak "         "ERROR"
run_test "dwie spacje między wyrazami"      "kayak  hello"   "ERROR"
run_test "cyfra w wyrazie"                  "kayak1"         "ERROR"
run_test "cyfra jako wyraz"                 "kayak 123"      "ERROR"
run_test "znak specjalny"                   "kayak!"         "ERROR"
run_test "znak specjalny myślnik"           "ka-yak"         "ERROR"
run_test "tylko spacja"                     " "              "ERROR"
run_test "tylko dwie spacje"               "  "              "ERROR"
run_test "tab w środku"                    "kayak	hello"    "ERROR"
run_test "spacja i \\n - spacja na końcu"   "kayak "         "ERROR"

echo ""
echo "--- Weryfikacja dokładnych bajtów odpowiedzi ---"
# Sprawdź że odpowiedź nie zawiera dodatkowych zerowych bajtów
actual_bytes=$(printf "%s" "kayak" | socat -t 2.0 stdio udp4:${SERVER}:${PORT} 2>/dev/null | od -A n -t u1 | tr -s ' ' | xargs)
expected_bytes="49 47 49"  # "1/1" w ASCII: 49='1', 47='/', 49='1'
if [ "$actual_bytes" = "$expected_bytes" ]; then
    echo "[PASS] odpowiedź '1/1' ma dokładnie 3 bajty (brak zer i \\r\\n)"
    ((PASS++))
else
    echo "[FAIL] odpowiedź '1/1' ma nieoczekiwane bajty: $actual_bytes"
    echo "       oczekiwano: $expected_bytes"
    ((FAIL++))
fi

actual_bytes=$(printf "%s" "1hello" | socat -t 2.0 stdio udp4:${SERVER}:${PORT} 2>/dev/null | od -A n -t u1 | tr -s ' ' | xargs)
expected_bytes="69 82 82 79 82"  # "ERROR" w ASCII
if [ "$actual_bytes" = "$expected_bytes" ]; then
    echo "[PASS] odpowiedź 'ERROR' ma dokładnie 5 bajtów (brak zer i \\r\\n)"
    ((PASS++))
else
    echo "[FAIL] odpowiedź 'ERROR' ma nieoczekiwane bajty: $actual_bytes"
    echo "       oczekiwano: $expected_bytes"
    ((FAIL++))
fi

echo ""
echo "========================================"
echo " Wynik: $PASS PASS, $FAIL FAIL"
echo "========================================"