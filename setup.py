# CarBuddy Python版 セットアップスクリプト
from setuptools import setup, find_packages

with open("README_PYTHON.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

with open("requirements.txt", "r", encoding="utf-8") as fh:
    requirements = [line.strip() for line in fh if line.strip() and not line.startswith("#")]

setup(
    name="carbuddy-python",
    version="1.0.0",
    author="CarBuddy Team",
    author_email="your-email@example.com",
    description="車載インタラクティブディスプレイシステム - Python版",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/yourusername/car-buddy",
    packages=find_packages(where="python"),
    package_dir={"": "python"},
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Topic :: System :: Hardware",
        "Topic :: Multimedia :: Graphics :: Presentation",
        "Topic :: Scientific/Engineering :: Interface Engine/Protocol Translator",
    ],
    python_requires=">=3.9",
    install_requires=requirements,
    extras_require={
        "dev": [
            "pytest>=7.0.0",
            "pytest-cov>=4.0.0",
            "pytest-mock>=3.7.0",
            "flake8>=5.0.0",
            "black>=22.0.0",
        ],
    },
    entry_points={
        "console_scripts": [
            "carbuddy=main:main",
        ],
    },
    include_package_data=True,
    package_data={
        "": ["*.md", "*.txt", "*.yaml", "*.yml"],
    },
    data_files=[
        ("images", ["images/README.md"]),
        ("", ["config.py", "requirements.txt"]),
    ],
)