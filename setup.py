from distutils.core import setup

files = ["/*"]

setup(
    name = "WhereIsItGoing",
    version = ".1",
    description = "",
    author = "Brian Scott and Scott Heinz"
    author_email = "bscott@csumb.edu",
    packages = ['WhereIsItGoing'],
    package_data = {'WhereIsItGoing' : files},
    scripts = ["runner"])
