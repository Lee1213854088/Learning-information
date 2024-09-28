# Define here the models for your scraped items
#
# See documentation in:
# https://docs.scrapy.org/en/latest/topics/items.html

#import scrapy


#class DoubanItem(scrapy.Item):
    # define the fields for your item here like:
    # name = scrapy.Field()
 #   pass


import scrapy
#"DoubanItem"是一个Scrapy的数据项（Item），用于存储网页上抓取的数据
class DoubanItem(scrapy.Item):
    title = scrapy.Field()   #标题
    rating = scrapy.Field()  #评分
    link = scrapy.Field()    #链接
    comments = scrapy.Field() #评论
