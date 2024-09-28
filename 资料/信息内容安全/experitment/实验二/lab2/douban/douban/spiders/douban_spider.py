#导入Scrapy框架，这是一个用于创建和管理Web爬虫的Python库
import scrapy
#导入名为"DoubanItem"的项目类，用于定义爬取到的数据的结构。
from douban.items import DoubanItem

#定义一个名为"DoubanSpider"的Spider类，继承自Scrapy的Spider类
class DoubanSpider(scrapy.Spider):
   # 设置Spider的名称为'douban_spider'，这个名称可以在运行爬虫时用来识别和调用爬虫。
   name = 'douban_spider'
   # 指定Spider的起始URL
   start_urls = ['https://movie.douban.com/top250']


#定义了用于处理起始URL响应的parse方法，它接收一个response参数，包含了来自起始URL的页面内容。
   def parse(self, response):
       # 获取电影列表
       movies = response.xpath('//ol[@class="grid_view"]/li')

       # 仅获取前10部电影
       for movie in movies[:10]:
           item = DoubanItem()
           item['title'] = movie.xpath('.//div[@class="hd"]/a/span[1]/text()').extract_first().strip()
           item['rating'] = movie.xpath(
               './/div[@class="bd"]/div[@class="star"]/span[2]/text()').extract_first().strip()
           item['link'] = movie.xpath('.//div[@class="hd"]/a/@href').extract_first()


          #发送一个新的请求，爬取电影的评论页面，并将之前创建的DoubanItem对象通过meta参数传递给parse_comments方法
           yield scrapy.Request(item['link'], callback=self.parse_comments, meta={'item': item})

   #定义了用于处理评论页面响应的parse_comments方法，它接收包含电影评论页面内容的response
   def parse_comments(self, response):
       #从之前请求中传递的meta中获取item，这个item包含了电影的基本信息
       item = response.meta['item']
       comments = response.xpath('//div[@class="comment"]/p/span[@class="short"]')

       # 获取前五热评
       #通过yield返回包含电影信息和评论的item对象，将它们传递给Scrapy管道进行后续处理
       item['comments'] = [comment.extract() for comment in comments[:5]]

       yield item