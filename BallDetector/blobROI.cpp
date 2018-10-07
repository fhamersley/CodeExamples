// The purpose of this function is to take ROI in from comboROI that are the wrong size or shape to include only a ball.
// The image is thresholded to produce a binary image that clearly shows the spots on the balls in a dynamically lit environment.
// calculateAdaptiveValuesForBlobROI() and preProcessAdaptive() acheive this.
// CCA is used on the binary image to find the black spots of the right size and density.
// The most dense blob is then used to select the set of blobs that are likely to be the other blobs on the ball.
// These selected blobs are then used to form a new set of several ROI/bdvb using the centre of blob for the centre of the new ROI
// and the midpoint/centroid of all the blobs, depending on how many blobs found. These are then pushed into the res vector
// This function greatly improved the ability of the detector to find the ball when it was included in a larger ROI of white such as
// field lines, goal posts and robots.

// Author: Fraser Hamersley for rUNSWift

bool BallDetector::blobROI(const VisionInfoIn& info_in, const RegionI& region, const VisionInfoMiddle& info_middle, VisionInfoOut& info_out, std::vector <BallDetectorVisionBundle> &res, RegionAspectType region_aspect_type) {
#ifdef BALL_DEBUG
            std::cout << "blobROI\n";
#endif // BALL_DEBUG

    bool bdvbAdded = false;

    // If we don't want to use it

    BallDetectorVisionBundle temp_bdvb;
    temp_bdvb.region = &region;
    temp_bdvb.original_region = &region;
    temp_bdvb.region_created = false;
    temp_bdvb.model_region_created = false;
    temp_bdvb.is_crazy_ball = false;

    // Copy RegionAspectType from comboROI
    temp_bdvb.region_aspect_type = region_aspect_type;

    // Amount to expand each region by
    int expand_by;

    getAverageBrightness(temp_bdvb);
    calculateAdaptiveValuesForBlobROI(temp_bdvb);
    preProcessAdaptive(temp_bdvb);

    processInternalRegionsROI(*temp_bdvb.region, temp_bdvb, temp_bdvb.internal_regions);

#ifdef BLOBROI_DEBUG
    std::vector<Point> region_centres;
    for (unsigned int i = 0; i < temp_bdvb.internal_regions.groups.size(); i++) {
        Point centre(((temp_bdvb.internal_regions.groups[i].max_x - temp_bdvb.internal_regions.groups[i].min_x + 1) / 2) + temp_bdvb.internal_regions.groups[i].min_x,
                     ((temp_bdvb.internal_regions.groups[i].max_y - temp_bdvb.internal_regions.groups[i].min_y + 1) / 2) + temp_bdvb.internal_regions.groups[i].min_y);
        region_centres.push_back(centre);
    }
    std::cout << "------------------- Old region centres --------------------\n";
    printRegionsAndCentres(*temp_bdvb.region, region_centres);
#endif //BLOBROI_DEBUG

    // Number of blobs we have to work with
    int number_of_blobs = temp_bdvb.internal_regions.groups.size();

    // If 1 or more but less than 6 regions found think about making a bdvb
    if (number_of_blobs > 0 && number_of_blobs < 7) {
        // Collect data about the most dense region. This assumes that this dense region is a ball blob (bb)
        // and that other bb will have similar size.
        // Comparison is done using the larger of the width or the height
        // as other bb will have a height or width similar

        int dense_reg_num = temp_bdvb.internal_regions.max_region_density_number - 1;        // -1 so it accesses the correct data in the vector
        int bb_size_x = temp_bdvb.internal_regions.groups[dense_reg_num].max_x -
                        temp_bdvb.internal_regions.groups[dense_reg_num].min_x + 1;          // +1 so the size is correct
        int bb_size_y = temp_bdvb.internal_regions.groups[dense_reg_num].max_y -
                        temp_bdvb.internal_regions.groups[dense_reg_num].min_y + 1;

        int blob_max_size = std::max(bb_size_x, bb_size_y);
        int blob_min = floor(blob_max_size * 0.7);
        int blob_max = ceil(blob_max_size * 1.3);

#ifdef BALL_DEBUG
        std::cout << "Dense region number: " << temp_bdvb.internal_regions.max_region_density_number

                  << " - width: " << bb_size_x << " height: " << bb_size_y << "\n";
        std::cout << "Allowable blob min: " << blob_min << " max: " << blob_max << "\n";
#endif // BALL_DEBUG

        // Find the region centres using the approx bb to throw out regions when we have more than 2
        std::vector<Point> region_centres;
        region_centres.reserve(8);
            for (std::vector<InternalRegion>::iterator it = temp_bdvb.internal_regions.groups.begin(); it != temp_bdvb.internal_regions.groups.end(); ++it) {
                int min_x = it->min_x;
                int min_y = it->min_y;
                int x_size = it->max_x - min_x + 1;         // +1 so the size is correct
                int y_size = it->max_y - min_y + 1;

                    // If more than 2 blobs then try to throw some out using the dense blob
                if (number_of_blobs > 2) {
                    // If the height or width of the blob are within the bounds of the likely ball blob store its centre
                    if (((x_size > blob_min) && (x_size < blob_max)) || ((y_size > blob_min) && (y_size < blob_max))) {
                        Point centre((x_size / 2) + min_x, (y_size / 2) + min_y);
                        region_centres.push_back(centre);
#ifdef BALL_DEBUG
                    std::cout << "Accepted blob - x size: " << x_size << " Y size: " << y_size << "\n";
                    }
                    else {
                        std::cout << "Rejected X size: " << x_size << " Y size: " << y_size << "\n";
#endif //BALL_DEBUG
                    }
                }
                // Don't throw any out
                else {
                    Point centre((x_size / 2) + min_x, (y_size / 2) + min_y);
                    region_centres.push_back(centre);
                }
            }
#ifdef BLOBROI_DEBUG
        std::cout << "------------------- New region centres --------------------\n";
        printRegionsAndCentres(*temp_bdvb.region, region_centres);
#endif //BLOBROI_DEBUG

        for (std::vector<Point>::iterator it = region_centres.begin(); it != region_centres.end(); ++it) {

            // If we don't want to use it
            BallDetectorVisionBundle bdvb;
            bdvb.region = &region;
            bdvb.original_region = &region;
            bdvb.region_created = false;
            bdvb.model_region_created = false;
            bdvb.is_crazy_ball = false;

            //Copy RegionAspectType from comboROI
            bdvb.region_aspect_type = region_aspect_type;

            // Blob is likely to be to the side of the ball so expand by x 2.5
            // unless it is the only blob and probably centred so expand by x 2
            if (region_centres.size() == 1) {
                expand_by = blob_max_size * 2;
            }
            else {
                expand_by = blob_max_size * 2.5;
            }

            BBox newBounds = BBox();
            newBounds.a.x() = it->x() - expand_by;
            newBounds.a.y() = it->y() - expand_by;
            newBounds.b.x() = it->x() + expand_by;
            newBounds.b.y() = it->y() + expand_by;

            BBox old = region.getBoundingBoxRaw();

            RegionI *blob_region = new RegionI(region, BBox(newBounds.a, newBounds.b), 1, 0, true);

            if (bdvb.region_created == true){
                delete bdvb.region;
            }

            bdvb.region = blob_region;
            bdvb.region_created = true;
            bdvbAdded = true;
            getSizeEst(bdvb, info_in, info_out);
            rescaleRegion(bdvb);
            res.push_back(bdvb);
        }
        // Push back extra bdvb using regions posistions to pick the likely centre
        if (region_centres.size() == 2) {

            // If we don't want to use it
            BallDetectorVisionBundle bdvb;
            bdvb.region = &region;
            bdvb.original_region = &region;
            bdvb.region_created = false;
            bdvb.model_region_created = false;
            bdvb.is_crazy_ball = false;

            //Copy RegionAspectType from comboROI
            bdvb.region_aspect_type = region_aspect_type;

            // If we have two regions use the midpoint
            Point midpoint = Point(((region_centres[0].x() + region_centres[1].x()) / 2),
                                   ((region_centres[0].y() + region_centres[1].y()) / 2));

            // Likely to be closer to the actual centre of the ball so expand by x 2
            int expand_by = blob_max_size * 2.0;

            BBox newBounds = BBox();
            newBounds.a.x() = midpoint.x() - expand_by;
            newBounds.a.y() = midpoint.y() - expand_by;
            newBounds.b.x() = midpoint.x() + expand_by;
            newBounds.b.y() = midpoint.y() + expand_by;

            RegionI *blob_region = new RegionI(region, BBox(newBounds.a, newBounds.b), 1, 0, true);

            if (bdvb.region_created == true){
                delete bdvb.region;
            }

            bdvb.region = blob_region;
            bdvb.region_created = true;
            bdvbAdded = true;
            getSizeEst(bdvb, info_in, info_out);

            rescaleRegion(bdvb);
            res.push_back(bdvb);
        }
        if (region_centres.size() == 3) {

            // If we don't want to use it
            BallDetectorVisionBundle bdvb;
            bdvb.region = &region;
            bdvb.original_region = &region;
            bdvb.region_created = false;
            bdvb.model_region_created = false;
            bdvb.is_crazy_ball = false;

            //Copy RegionAspectType from comboROI
            bdvb.region_aspect_type = region_aspect_type;

            Point centroid = Point(((region_centres[0].x() + region_centres[1].x() + region_centres[2].x()) / 3),
                                   ((region_centres[0].y() + region_centres[1].y() + region_centres[2].y()) / 3));

            // Likely to be closer to the actual centre of the ball so expand by x 2
            int expand_by = blob_max_size * 2.0;

            BBox newBounds = BBox();
            newBounds.a.x() = centroid.x() - expand_by;
            newBounds.a.y() = centroid.y() - expand_by;
            newBounds.b.x() = centroid.x() + expand_by;
            newBounds.b.y() = centroid.y() + expand_by;

            RegionI *blob_region = new RegionI(region, BBox(newBounds.a, newBounds.b), 1, 0, true);

            if (bdvb.region_created == true){
                delete bdvb.region;
            }

            bdvb.region = blob_region;
            bdvb.region_created = true;
            bdvbAdded = true;
            getSizeEst(bdvb, info_in, info_out);

            rescaleRegion(bdvb);
            res.push_back(bdvb);
        }
    }
    if (temp_bdvb.region_created == true) {
        delete temp_bdvb.region;
    }
    return bdvbAdded;
}