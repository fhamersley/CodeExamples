// Functions for calculating dynamic adaptive thresholding parameters based off the calculated average brightness of the images

// Author: Fraser Hamersley for rUNSWift

void BallDetector::calculateAdaptiveValuesForCircleFitting(BallDetectorVisionBundle &bdvb)
{
    int rows = bdvb.region->getRows();
    int win, per = 0;
    if (bdvb.region->isTopCamera()) {
        win = rows * 0.40;
        per = -5;
    }
    else {
        win = rows * 0.40;
        per = -5;
    }
    // If window size is negtive it will use the last correct window size
    if (win <= 0) {
        bdvb.window_size = DEFAULT_ADAPTIVE_THRESHOLDING_WINDOW_SIZE;   // Dummy value
    }
    else {
        bdvb.window_size = win;
    }
    bdvb.percentage = per;
#ifdef BALL_DEBUG
    std::cout << "Calc AT Values for CF -\n";
    std::cout << "Window Size: " << bdvb.window_size << " Percentage: " << bdvb.percentage << "\n";
#endif //BALL_DEBUG
}

void BallDetector::calculateAdaptiveValuesForInternalRegions(BallDetectorVisionBundle &bdvb)
{
    int rows = bdvb.region->getRows();
    int win, per = 0;
    if (bdvb.region->isTopCamera()) {
        win = rows * 0.30;
        per = int(bdvb.avg_brightness / 10);
    }
    else {
        win = rows * 0.50;
        per = 10;
    }
    // If window size is negtive it will use the last correct window size
    if (win <= 0) {
        bdvb.window_size = DEFAULT_ADAPTIVE_THRESHOLDING_WINDOW_SIZE;   // Dummy value
    }
    else {
        bdvb.window_size = win;
    }
    bdvb.percentage = per;
#ifdef BALL_DEBUG
    std::cout << "Calc AT Values for IR -\n";
    std::cout << "Window Size: " << bdvb.window_size << " Percentage: " << bdvb.percentage << "\n";
#endif //BALL_DEBUG
}

void BallDetector::calculateAdaptiveValuesForBlobROI(BallDetectorVisionBundle &bdvb)
{
    int rows = bdvb.region->getRows();
    int cols = bdvb.region->getCols();

    // Assuming the region that has the potential ball is as wide OR high as the ball
    int val = std::min(rows, cols);
    int win, per = 0;
    if (bdvb.region->isTopCamera()) {
        win = val * 0.50;
        per = int(bdvb.avg_brightness / 10);
    }
    else {
        win = val * 0.60;
        per = 10;
    }
    // If window size is negtive it will use the last correct window size
    if (win <= 0) {
        bdvb.window_size = DEFAULT_ADAPTIVE_THRESHOLDING_WINDOW_SIZE;   // Dummy value
    }
    else {
        bdvb.window_size = win;
    }
    bdvb.percentage = per;
#ifdef BALL_DEBUG
    std::cout << "Calc AT Values for blobROI -\n";
    std::cout << "Window Size: " << bdvb.window_size << " Percentage: " << bdvb.percentage << "\n";
#endif //BALL_DEBUG
}