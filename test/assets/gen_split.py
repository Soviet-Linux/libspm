import os
import sys
import random
import string

if len(sys.argv) < 3:
    print("Usage: python gen_split.py <file> <length>")
    sys.exit(1)

file = sys.argv[1]
length = int(sys.argv[2])

# generate random string with ASCII chars, digits, and commas of length sys.argv[2]
random_string = ''
last_char = ''
for _ in range(length):
    next_char = random.choice(string.ascii_letters + string.digits + ',')
    while last_char == ',' and next_char == ',':
        next_char = random.choice(string.ascii_letters + string.digits + ',')
    random_string += next_char
    last_char = next_char

# write it to file
with open(file, 'w') as f:
    f.write(random_string)