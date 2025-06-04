import serial
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt

# === Initialize Serial Port ===
ser = serial.Serial('/dev/tty.usbmodem101', timeout=1)
print('Opening port:', ser.name)

# === Image dimensions ===
WIDTH = 80
HEIGHT = 60
reds = np.zeros((HEIGHT, WIDTH), dtype=np.uint8)
greens = np.zeros((HEIGHT, WIDTH), dtype=np.uint8)
blues = np.zeros((HEIGHT, WIDTH), dtype=np.uint8)

while True:
    selection = input('\nENTER COMMAND ("c" to capture, "q" to quit): ')
    if selection == 'q':
        print('Exiting client.')
        ser.close()
        break

    if selection == 'c':
        ser.reset_input_buffer()
        ser.write(b'c\n')

        count = 0
        com_value = None

        while count < WIDTH * HEIGHT or com_value is None:
            line = ser.readline()
            try:
                line = line.decode().strip()
                if line.startswith("COM:"):
                    com_value = int(line.split(":")[1].strip())
                else:
                    i, r, g, b = map(int, line.split())
                    row = i // WIDTH
                    col = i % WIDTH
                    reds[row][col] = r
                    greens[row][col] = g
                    blues[row][col] = b
                    count += 1
            except Exception as e:
                print(f"Skipping invalid line: {line} ({e})")
                continue

        # === Display the image ===
        rgb_array = np.stack((reds, greens, blues), axis=-1)
        image = Image.fromarray(rgb_array)
        plt.imshow(image)
        plt.axis("off")
        plt.title(f"Captured Image — COM = {com_value}")
        plt.show()

        print(f"Center of Mass (COM): {com_value}")
    else:
        print("Invalid command. Use 'c' to capture or 'q' to quit.")
