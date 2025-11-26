import csv
import numpy as np

def load_csv(path):
    times = []
    ch0 = []
    ch2 = []

    with open(path, newline='') as f:
        reader = csv.DictReader(f, delimiter='\t')
        for row in reader:
            times.append(float(row['Time [s]']))
            ch0.append(int(row['Channel 0']))
            ch2.append(int(row['Channel 2']))

    return np.array(times), np.array(ch0), np.array(ch2)


def find_transitions(times, signal):
    """Retorna índices onde há mudança de nível."""
    transitions = []
    for i in range(1, len(signal)):
        if signal[i] != signal[i-1]:
            transitions.append(i)
    return transitions


def compute_jitter(times, ch0, ch2):
    idx_transitions = find_transitions(times, ch0)
    jitters = []

    for idx in idx_transitions:
        target_level = ch0[idx]
        t0 = times[idx]

        # Procurar à frente o momento em que ch2 vira igual a ch0
        found = False
        for j in range(idx, len(ch2)):
            if ch2[j] == target_level:
                t2 = times[j]
                jitters.append(t2 - t0)
                found = True
                break

        if not found:
            jitters.append(np.nan)

    return np.array(jitters), idx_transitions


def main():
    times, ch0, ch2 = load_csv("logic_data/digital.csv")
    jitters, transitions = compute_jitter(times, ch0, ch2)

    print("---- JITTER RESULTS ----")
    print(f"Total transitions analisadas: {len(transitions)}")
    print(f"Jitter médio : {np.nanmean(jitters)*1e6:.2f} us")
    print(f"Jitter mínimo: {np.nanmin(jitters)*1e6:.2f} us")
    print(f"Jitter máximo: {np.nanmax(jitters)*1e6:.2f} us")
    
    print("\nLista completa de jitters (em us):")
    print(jitters * 1e6)


if __name__ == "__main__":
    main()