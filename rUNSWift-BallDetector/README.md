## Ball Detector

These functions are all used in ball detector pipe line and are member functions of the class. They have been put into individual .cpp files for readability. The functions included were explicitly written by myself, though I was involved in almost every stage of the ball detector outlined below, through improvements to precision and recall as well as timing optimisations.

The detector is for a 10cm wide black and white 'traditional' soccer ball design. Each function has had a description added at the top to explain its purpose in the pipeline to give context. The inline comments have not been edited otherwise. Some nomenclature can be found at the bottom of this readme.

A basic overview of the pipeline is as follows.

1. Regions of interest (ROI) are passed into the detector in RegionI format. These ROI are formed from a binary thesholded image where regions of white pixels produce these ROI. Large regions of white are broken into smaller segments.
2. The detector iterates through these ROI individually until all have been inspected or ball is found.
3. Each region is inspected in comboROI for simple heuristics such as aspect ratio and esitmated size (through kinematics) and then passed onto ROI processors such as BlobROI. These ROI processors prepare the image for the rest of the detector through zooming and resizing so that the potential ball is the only feature in the image.
4. The ROI is rethresholded to produce a binary image suitable for circle fitting using the average brightness of the image.
5. Candidate points for circle fitting are produced by scanning linearly looking for the transition from black to white.
6. Circle is fitted.
7. The ROI is rethresholded to produce a binary image suitable for finding the black patches on the ball.
8. Black patches are found within the fitted circle using CCA and analysed for heuristics such as number, relative size and if they border the fitted circle.
9. Equilateral triangles of the correct relative size are attemped to be formed between patch using all possible combinations.
10. After passing all previous heuristic checks the final stage of the ball detector is a gaussian mixed model classifier.

Nomenclature: 

RegionI - An image processing handle that provides access to the underlying data for that image including its raw image data, binary (known as colour) thresholded image data, position, size and density in terms of the original full image and more. RegionI constructors also allow for a RegionI passed in to be resized, have it's density changed (zoom in/out), for it to be thresholded again using different adaptive thresholding values.

bdvb (ball detector vision bundle) - A bundle of data relating to the current ROI being inspected and includes it's RegionI. Used to pass data through the detector pipeline.

CCA - Connected component analysis.

AT - Adaptive thresholding
