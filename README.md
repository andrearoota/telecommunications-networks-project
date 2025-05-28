# Telecommunications Networks Project

## Course Information

This project is part of the Master's Degree in Computer Engineering course, specifically for the "Telecommunications Networks" class taught by Professor Fabio Martignon.

## Assignment

- Consider a connection between two nodes modeled as a queueing system with a single server and infinite queue capacity.  
Arrivals follow a Poisson process with rate 𝜆, and service times are exponentially distributed with mean 1/𝜇.

- During transmission, an error may occur with probability *p*.  
The receiver sends acknowledgments through a separate channel at Poisson-distributed times with rate 𝛿.  
The sender uses a stop-and-wait protocol: it sends one packet and waits for the acknowledgment before proceeding.

- The software receives all the parameters as input and computes the **average traversal time**.

## Usage

1. **Compile the project**  
 Use the `make` command in the project directory:
 ```bash
 make
```
   
2. **Run the executable**
  Once compiled, start the program with:
```bash
 ./queue
```

## Contribution

Feel free to fork the repository and submit pull requests. If you encounter any issues or have suggestions, please open an issue in the GitHub repository.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.
