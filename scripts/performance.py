import requests
import time
import threading

SERVER_URL = "http://127.0.0.1:10339"

TEST_FILE = "scripts/test_image.jpg"

NUM_THREADS = 1

REQUESTS_PER_THREAD = 50

def test_get_file():
    url = f"{SERVER_URL}/file/{TEST_FILE}"
    for _ in range(REQUESTS_PER_THREAD):
        try:
            response = requests.get(url)
            if response.status_code == 200:
                print(f"GET {url} - Success")
            else:
                print(f"GET {url} - Failed with status {response.status_code}")
        except Exception as e:
            print(f"GET {url} - Error: {e}")


def test_post_upload():
    url = f"{SERVER_URL}/upload"
    headers = {
        'Content-Type': 'multipart/form-data',
    }

    for _ in range(REQUESTS_PER_THREAD):
        try:
            files=[
                ('file',('test_image.jpg',open(TEST_FILE,'rb'),'image/jpeg'))
            ]
            response = requests.post(url, files=files, headers=headers)
            if response.status_code == 200:
                print(f"POST {url} - Success")
            else:
                print(f"POST {url} - Failed with status {response.status_code}")
                print(f"response body: {response.text}")
        except Exception as e:
            print(f"POST {url} - Error: {e}")

        # time.sleep(2)


def run_test(test_function):
    """运行测试函数"""
    threads = []
    start_time = time.time()

    # 创建线程
    for _ in range(NUM_THREADS):
        thread = threading.Thread(target=test_function)
        threads.append(thread)
        thread.start()

    # 等待所有线程完成
    for thread in threads:
        thread.join()

    end_time = time.time()
    print(f"Test completed in {end_time - start_time:.2f} seconds")


if __name__ == "__main__":
    print("Testing GET requests...")
    run_test(test_get_file)

    print("Testing POST requests...")
    run_test(test_post_upload)