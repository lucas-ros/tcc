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
# Detecta bordas (subida e descida)
# ---------------------------------------------------------
def detect_edges(times, signal):
    """Retorna arrays com timestamps das bordas de subida e descida."""
    rising = []
    falling = []

    for i in range(1, len(signal)):
        if signal[i-1] == 0 and signal[i] == 1:
            rising.append(times[i])
        elif signal[i-1] == 1 and signal[i] == 0:
            falling.append(times[i])

    return np.array(rising), np.array(falling)


# ---------------------------------------------------------
# Remove 5% das menores e maiores latências
# ---------------------------------------------------------
def trim_edges(arr, p=0.05):
    if len(arr) == 0:
        return arr

    arr_sorted = np.sort(arr)
    k = int(len(arr) * p)

    # Se não houver elementos suficientes para recorte, apenas retorna
    if k == 0 or len(arr_sorted) < 2*k + 1:
        return arr_sorted

    return arr_sorted[k:-k]


# ---------------------------------------------------------
# Pareamento e cálculo de latência baseado em bordas
# ---------------------------------------------------------
def compute_latency_edges(times, ch0, ch2):

    r0, f0 = detect_edges(times, ch0)
    r2, f2 = detect_edges(times, ch2)

    def match_edges(e0, e2):
        latencies = []
        j = 0

        for t0 in e0:
            # Avança no canal 2 até achar o primeiro evento >= t0
            while j < len(e2) and e2[j] < t0:
                j += 1
            if j < len(e2):
                latencies.append(e2[j] - t0)
                j += 1

        return np.array(latencies)

    # Rising edges
    lat_rising = match_edges(r0, r2)
    # Falling edges
    lat_falling = match_edges(f0, f2)

    # Junta tudo
    lat_all = np.concatenate([lat_rising, lat_falling])

    # Filtra erros: negativos ou zero (ruído ou coincidência inválida)
    lat_all = lat_all[lat_all > 0]

    # ---- AQUI entra a remoção de 5% ----
    lat_all = trim_edges(lat_all, p=0.05)

    return lat_all, lat_rising, lat_falling


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
    
    lat_all, lat_rising, lat_falling = compute_latency_edges(times, ch0, ch2)

    if len(lat_all) == 0:
        print("Nenhuma latência válida encontrada. Verifique os sinais.")
        return

    lat_media = np.mean(lat_all)
    jitter = np.std(lat_all)
    lat_min = np.min(lat_all)
    lat_max = np.max(lat_all)

    filename = create_output_filename()

    with open(filename, "w") as f:
        f.write("---- LATÊNCIA / JITTER (baseado em bordas) ----\n\n")
        f.write(f"Total de bordas pareadas (após trimming 5%): {len(lat_all)}\n\n")
        f.write(f"Latência média : {lat_media*1e6:.6f} us\n")
        f.write(f"Jitter (std)   : {jitter*1e6:.6f} us\n")
        f.write(f"Latência mínima: {lat_min*1e6:.6f} us\n")
        f.write(f"Latência máxima: {lat_max*1e6:.6f} us\n")

    print(f"Arquivo gerado: {filename}")


if __name__ == "__main__":
    main()
