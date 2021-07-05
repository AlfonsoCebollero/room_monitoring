import requests as req

key = "CCEeLL1HMUiCvd6Ejw7SEQ"
res = req.get("https://www.goodreads.com/book/review_counts.json", params={"key": key, "isbns": "9781632168146"})
print(res.json())