import music

import numpy as np
import scipy.io


def speedx(snd_array, factor):
    """ Speeds up / slows down a sound, by some factor. """
    indices = np.round(np.arange(0, len(snd_array), factor))
    indices = indices[indices < len(snd_array)].astype(int)
    return snd_array[indices]


def stretch(snd_array, factor, window_size, h):
    """ Stretches/shortens a sound, by some factor. """
    phase = np.zeros(window_size)
    hanning_window = np.hanning(window_size)
    result = np.zeros(int(len(snd_array) / factor + window_size))

    for i in np.arange(0, len(snd_array) - (window_size + h), h * factor):
        i = int(i)
        # Two potentially overlapping subarrays
        a1 = snd_array[i : i + window_size]
        a2 = snd_array[i + h : i + window_size + h]

        # The spectra of these arrays
        s1 = np.fft.fft(hanning_window * a1)
        s2 = np.fft.fft(hanning_window * a2)

        # Rephase all frequencies
        phase = (phase + np.angle(s2 / s1)) % 2 * np.pi

        a2_rephased = np.fft.ifft(np.abs(s2) * np.exp(1j * phase))
        i2 = int(i / factor)
        result[i2 : i2 + window_size] += hanning_window * a2_rephased.real

    # normalize (16bit)
    result = (2 ** (16 - 4)) * result / result.max()

    return result.astype("int16")


def pitchshift(snd_array, n, window_size=2 ** 13, h=2 ** 11):
    """ Changes the pitch of a sound by ``n`` semitones. """
    factor = 2 ** (1.0 * n / 12.0)
    stretched = stretch(snd_array, 1.0 / factor, window_size, h)
    return speedx(stretched[window_size:], factor)


import os
import glob
import tqdm


def main():
    for f in tqdm.tqdm(glob.glob("outputs/*/*.wav")):
        # print(f)
        nd = f.replace(".wav", "")
        os.makedirs(nd, exist_ok=True)
        (_, data) = scipy.io.wavfile.read(f)
        fft = abs(np.fft.rfft(data))
        max_freq = np.argmax(fft)
        scaling_factor = 27.5 / (max_freq)
        semitones = np.log2(scaling_factor) * 12
        # print(max_freq, scaling_factor, semitones)
        for (i, s) in enumerate(np.arange(semitones, semitones + 88, 1), start=1):
            out_array = pitchshift(data, s)
            out_array = out_array.astype(float)
            out_array /= out_array.max()
            # print(out_array.shape, out_array.max(), end=" ")
            scipy.io.wavfile.write(
                filename=nd + "/" + str(i) + ".wav", rate=16000, data=out_array
            )
        # print()


if __name__ == "__main__":
    main()
