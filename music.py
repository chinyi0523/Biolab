import glob
import json
import os

import librosa
import matplotlib.pyplot as plt
import numpy as np
import scipy.io


class Serial(object):
    def __call__(self, data: np.ndarray):
        avg_freq = self.fft(arr=data, n_fft=1600, hop_length=200, win_length=1000)
        # avg_freq = self.pad(avg_freq, 16000)
        coeff = dict(enumerate(json.load(open("coeff.json"))))
        avg_freq = self.repeat(avg_freq, coeff)
        time_wav = np.fft.irfft(avg_freq)[2400:-2400]
        return time_wav / np.max(time_wav).item()

    @staticmethod
    def fft(
        arr: np.ndarray, n_fft: int, hop_length: int, win_length: int, *args, **kwargs
    ) -> np.ndarray:
        abs_fft_list = list()
        ang_fft_list = list()
        for index in range(0, len(arr) - n_fft, hop_length):
            to_fft = np.fft.fft(arr[index : index + n_fft])
            ang_fft = np.angle(to_fft)
            abs_fft_list.append(abs(to_fft))
            ang_fft_list.append(ang_fft)
        abs_fft = sum(abs_fft_list) / len(abs_fft_list)
        ang_fft = sum(ang_fft_list) / len(ang_fft_list)
        return abs_fft * np.exp(1j * ang_fft)

    @staticmethod
    def repeat(arr: np.ndarray, coeff: dict):
        array = list()
        for i in range(len(coeff)):
            array.append(arr / coeff[i])
        return np.concatenate(array, axis=0)

    @staticmethod
    def pad(arr: np.ndarray, to: int):
        assert arr.ndim == 1
        return np.concatenate((arr, np.zeros(shape=(to - len(arr),))), axis=0)


if __name__ == "__main__":
    names = ["all", "alpha", "beta", "theta", "delta"]
    all_files = glob.glob("*/*.mat")
    out_dir = "outputs"
    os.makedirs(out_dir, exist_ok=True)
    for f in all_files:
        mat_data = scipy.io.loadmat(file_name=f)["data"].T
        of = f.replace(".mat", "")
        (directory, file) = os.path.split(of)
        os.makedirs(f"{out_dir}/{directory}", exist_ok=True)
        for i in range(5):
            data = Serial()(data=mat_data[i])
            scipy.io.wavfile.write(
                filename=f"{out_dir}/{of}_{names[i]}.wav", rate=16000, data=data
            )
