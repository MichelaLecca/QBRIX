# QBRIX
An implementation of "QBRIX : a quantile-based approach to Retinex" by Gianini, Manenti and Rizzi (JOSA A, 2014) 

This code implements the Milano Retinex algorithm QBRIX for enhancing color images, published here:
Gabriele Gianini, Andrea Manenti, and Alessandro Rizzi, "QBRIX: a quantile-based approach to Retinex," J. Opt. Soc. Am. A 31, 2663-2673 (2014), DOI:10.1364/JOSAA.31.002663


QBRIX takes as input a color image, processes its color channels independently and outputs a new image, which is an enhanced version of the input one. Precisely, for each channel, QBRIX re-scales the intensity of each pixel by a value computed from a quantile (specified by the user) of the channel distribution, possibly weighted by a function depending on the position of the pixel with respect to its neighbors. The ouput image has higher brightness and contrast, a better color distribution and pssobly color dominants due to the illumination are mitigated or even removed.

There are two modalities:

1) Global QBRIX: the quantile specified by the user is computed from the channel distribution (no weights are used);

2) Local QBRIX: the quantile specified by the user varies from pixel to pixel. Given a pixel x, the quantile used by the algorithm is computed from the channel distribution where each intensity is weighted by a function inversely proportional to the Euclidean distance from the pixel x. 

The code works as follows:

QBRIX <input_image> <quantile> <mode> <dir_out>

where <input_image> is the image to be enhanced; <quantile> is the quantile to be specified for the enhancement; <mode> is -1 for Global QBRIX, while is 1 for Local QBRIX; <dir_out> is the directory where to save the output image. 
The value <quantile> ranges over [0, 100], but recommended values for this quantile are greater or equal than 90.
Too low values are not recommended, since thay may generate saturation.  When the quantile equals 100, QBRIX implements the scale-by-max algorithm, which is a limit case of Retinex.

The complexity of Local QBRIX is quadratic with the number of image pixels, thus long execution time is expected on large images. 



