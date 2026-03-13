import socket
import speech_recognition as sr
import io
import wave
import numpy as np  # Required for gain boosting

# Configuration
UDP_IP = "0.0.0.0"
UDP_PORT = 3333
SAMPLE_RATE = 16000
GAIN_FACTOR = 5.0  # Adjust this: 2.0 = 2x volume, 10.0 = 10x volume

# Initialize recognizer
recognizer = sr.Recognizer()

# Setup UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening with {GAIN_FACTOR}x gain on port {UDP_PORT}...")

def boost_gain(raw_data, factor):
    # Convert raw bytes to 16-bit integers
    audio_data = np.frombuffer(raw_data, dtype=np.int16).astype(np.float32)
    # Multiply by gain factor
    audio_data *= factor
    # Clip to prevent overflow distortion
    audio_data = np.clip(audio_data, -32768, 32767)
    return audio_data.astype(np.int16).tobytes()

def transcribe_audio(raw_data):
    # Apply the volume boost before transcription
    boosted_data = boost_gain(raw_data, GAIN_FACTOR)
    
    with io.BytesIO() as wav_io:
        with wave.open(wav_io, 'wb') as wav_file:
            wav_file.setnchannels(1)
            wav_file.setsampwidth(2)
            wav_file.setframerate(SAMPLE_RATE)
            wav_file.writeframes(boosted_data)
        
        wav_io.seek(0)
        with sr.AudioFile(wav_io) as source:
            audio = recognizer.record(source)
            try:
                text = recognizer.recognize_google(audio)
                print(f"Transcribed: {text}")
            except sr.UnknownValueError:
                pass 
            except sr.RequestError as e:
                print(f"API Error: {e}")

audio_buffer = bytearray()
while True:
    data, addr = sock.recvfrom(2048)
    audio_buffer.extend(data)

    # Transcribe every ~5 seconds (160,000 bytes)
    if len(audio_buffer) > 160000:
        transcribe_audio(bytes(audio_buffer))
        audio_buffer.clear()
