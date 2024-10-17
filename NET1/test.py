import requests

# 使用代理
proxy = { "http": "127.0.0.1:808" }

while True:
    URL = input('请输入网址：')

    # 打印 requests 库获取到的结果
    # 会返回网页 HTML 代码

    response = requests.get(URL, proxies = proxy)

    
    # 查看返回结果的状态码
    print(response.status_code)

    # 查看返回结果的内容，即网页源代码
    print(response.text)