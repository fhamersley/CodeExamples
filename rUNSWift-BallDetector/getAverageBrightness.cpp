// Function for calulating the average brightness of the given ROI. 
// Scans through the binary image summing the Y channel of the raw image for each previously thresholded white pixel.
// Then calulates the average.
// Average brightness is used to determine adaptive thresholding values for dynamic lighting conditions

// Author: Fraser Hamersley for rUNSWift

void BallDetector::getAverageBrightness(BallDetectorVisionBundle &bdvb)
{
    RegionI::iterator_fovea cur_point_fov = bdvb.region->begin_fovea();
    RegionI::iterator_raw cur_point_raw = bdvb.region->begin_raw();

    // The number of rows and columns in the region.
    int rows = bdvb.region->getRows();
    int cols = bdvb.region->getCols();

    // Data
    int whites = 0;
    int total_raw = 0;

    // Loop
    for(int pixel = 0; pixel < cols*rows; ++pixel)
    {
        if (cur_point_fov.colour() == 1) {
            total_raw += int(*cur_point_raw.raw());
            whites++;
        }
        ++cur_point_fov;
        ++cur_point_raw;
    }
    if (whites == 0) {
        whites++;
    }
    // Store
    bdvb.avg_brightness = total_raw / whites;

#ifdef BALL_DEBUG
    std::cout << "Whites: " << whites << "Total: " << total_raw << "\n";
    std::cout << "AVG: " << bdvb.avg_brightness << "\n";
#endif // BALL_DEBUG
}