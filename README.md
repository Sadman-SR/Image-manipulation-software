# Image-manipulation-software


A C-based desktop application for image processing, built using the IUP GUI toolkit. This project allows users to load ` 24 bit uncompressed.bmp` images, apply various transformations and filters, and save the edited results.

## Features

* **File Operations:** Open and Save `.bmp` image files.
* **Basic Editing:** Undo last action, revert to original image.
* **Transformations:** 
  * Rotate 90 degrees clockwise
  * Flip Horizontally & Vertically
  * Crop to specific coordinates
* **Filters & Adjustments:**
  * Apply Grayscale
  * Adjust Brightness (0 to 255)
  * Invert Colors
  * 3x3 Neighborhood Blur (Kernel-less implementation)
 
 ## File Structure
 
  * main.c: Application entry point.
  * gui.c / gui.h: IUP GUI layout, controls, and callback handlers.
  * image.c / image.h: Image memory allocation, pixel structures, and BMP file I/O.
  * process.c / process.h: Core pixel processing algorithms in pure C.
  * iup_tmp: Bundled IUP header and library files.
  * Makefile : Automated build script configured for WSL/Linux environments.
  
 ## Dependencies
This project uses **IUP v3.31** for the GUI. The library ZIP (downloaded via WSL) is included in this repository strictly for compilation convenience; all rights belong to its original creators at Tecgraf/PUC-Rio.
  
## Screenshots
### 1. Main Interface
![Main Interface](assets/interface.png)
### 2. Image Loaded
![Image Loaded](assets/loaded_img.png)
### 3. Applying Grayscale
![Applying Grayscale](assets/gray.png)
### 4. Applying blur 
![Applying blur](assets/blur.png)
### 5. Adjusting Brightness
![Adjusting Brightness](assets/decressed_bright.png)


## How to Compile and Run

### Prerequisites

* **Linux / WSL:** Install `gcc`, `make`, and `libgtk-3-dev`:
  ```bash
  sudo apt update && sudo apt install -y build-essential libgtk-3-dev make

To launch the application:
`./image_editor`

