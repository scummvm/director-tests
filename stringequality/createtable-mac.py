if __name__ == "__main__":
    with open("D3-mac-equalitytable.txt", "r") as file:
        data = file.read()
        table_from_mac = [i for i in data.replace("-", "").replace("\"","").replace(" ", "").split('\n') if i]
        equality_table = list(range(256))
        for i in table_from_mac:
            a, b  = i.split(":")
            equality_table[int(a)] = int(b)
        print([hex(i) for i in equality_table])
