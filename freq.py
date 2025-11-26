import csv
import numpy as np

def load_csv(path):
    times = []
    ch0 = []

    with open(path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            times.append(float(row['Time [s]']))
            ch0.append(int(row['Channel 0']))

    return np.array(times), np.array(ch0)


def count_edges(signal):
    edges = 0
    for i in range(1, len(signal)):
        if signal[i] != signal[i - 1]:
            edges += 1
    return edges


def main():
    times, ch0 = load_csv("logic_data/digital.csv")

    edges = count_edges(ch0)
    duration = times[-1] - times[0]

    # Cada período tem *duas* bordas (subida + descida)
    freq = (edges / 2) / duration

    print(f"Total de bordas: {edges}")
    print(f"Duração        : {duration:.6f} s")
    print(f"Frequência     : {freq:.3f} Hz")


if __name__ == "__main__":
    main()
