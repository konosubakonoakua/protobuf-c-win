import click
from os import system


@click.command()
@click.option("--file", default=".\*.proto", help="Proto file full path.")
@click.option("--bin", default=".\protoc.exe", help="Protoc.exe binary file full path.")
@click.option("--dir", default=".", help="Output file full path.")
def gen(file, bin, dir):
    test = ""
    test = system(bin + " --c_out=" + dir + " " + file)
    if test is 0:
        print("C-Header & C-File Generated.")
    else:
        print('Check your file path, try to use "\\" instead of "/" on Win platform please.')


if __name__ == '__main__':
    gen()