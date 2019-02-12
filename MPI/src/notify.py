import urllib2 as rr
from env import url,phone,url_mail,mail

def send_phone_notify():
    r = rr.urlopen(url+phone).read()

def send_mail_notify():
    r = rr.urlopen(url_mail+mail).read()
