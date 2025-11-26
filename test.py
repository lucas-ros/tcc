import csv
import numpy as np
import time
import os

# ---------------------------------------------------------
# Carrega CSV
# ---------------------------------------------------------
def load_csv(path):
    times = []
    ch0 = []
    ch2 = []

    with open(path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            times.append(float(row['Time [s]']))
            ch0.append(int(row['Channel 0']))
            ch2.append(int(row['Channel 2']))

    return np.array(times), np.array(ch0), np.array(ch2)


# ---------------------------------------------------------
# Detecta transições
# ---------------------------------------------------------
def find_transitions(signal):
    transitions = []
    for i in range(1, len(signal)):
        if signal[i] != signal[i - 1]:
            transitions.append(i)
    return transitions


# ---------------------------------------------------------
# Calcula latência média + jitter
# ---------------------------------------------------------
def compute_latency_and_jitter(times, ch0, ch2):
    idx_trans = find_transitions(ch0)
    diffs = []

    for idx in idx_trans:
        target_level = ch0[idx]
        t0 = times[idx]

        for j in range(idx, len(ch2)):
            if ch2[j] == target_level:
                diffs.append(times[j] - t0)
                break

    diffs = np.array(diffs)

    latencia_media = np.mean(diffs)
    jitter = np.std(diffs)

    return latencia_media, jitter, diffs, idx_trans


# ---------------------------------------------------------
# Gera nome único de arquivo
# ---------------------------------------------------------
def create_output_filename():
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    return f"resultados_jitter_{timestamp}.txt"


# ---------------------------------------------------------
# Main
# ---------------------------------------------------------
def main():
    times, ch0, ch2 = load_csv("digital.csv")
    lat, jit, diffs, transitions = compute_latency_and_jitter(times, ch0, ch2)

    filename = create_output_filename()

    with open(filename, "w") as f:
        f.write("---- LATÊNCIA / JITTER ----\n")
        f.write(f"Transições: {len(transitions)}\n")
        f.write(f"Latência média : {lat*1e6:.6f} us\n")
        f.write(f"Jitter (std)   : {jit*1e6:.6f} us\n\n")

        f.write("Primeiras 20 diferenças (us):\n")
        for v in (diffs[:20] * 1e6):
            f.write(f"{v:.6f}\n")

    print(f"Arquivo gerado: {filename}")


if __name__ == "__main__":
    main()
