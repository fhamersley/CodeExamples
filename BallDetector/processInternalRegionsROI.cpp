// Calls the CCA function and then processes the results for use in blobROI.
// CCA stores in member data vectors: number of pixels in blob i (group_counts_[i]), position (group_low/high_x/ys_[i])
// Inspects the blobs (group) for aspect ratio and density removing any unsuitable to be ball spots
// Also finds the most dense blob for use in blobROI.

// Author: Fraser Hamersley for rUNSWift

void BallDetector::processInternalRegionsROI(const RegionI &base_region, BallDetectorVisionBundle &bdvb,
     InternalRegionFeatures &internal_regions) {

    connectedComponentAnalysisNotWhite(base_region, bdvb);

    InternalRegionFeatures internal_region_features;
    internal_region_features.num_internal_regions = 0;
    internal_region_features.num_regions = 0;
    internal_region_features.max_internal_region_prop = 0;
    internal_region_features.groups.reserve(10);

    int min_internal_group_size, max_internal_group_size;

    // Max and min pix in a blob
    min_internal_group_size = 6;
    max_internal_group_size = 200;

    // For finding the most dence region
    int density_error = 100;
    int cur_den_err;

    // Count the number of groups that might be blobby.
    for(int group=0; group<group_links_.size(); ++group)
    {
        if(group_counts_[group] > min_internal_group_size &&
                group_counts_[group] < max_internal_group_size)
        {
            InternalRegion r;
            r.num_pixels = group_counts_[group];
            r.min_x = group_low_xs_[group];
            r.max_x = group_high_xs_[group];
            r.min_y = group_low_ys_[group];
            r.max_y = group_high_ys_[group];

            float x_size = (group_high_xs_[group] - group_low_xs_[group]);
            float y_size = (group_high_ys_[group] - group_low_ys_[group]);

            //If the aspect ratio of the blob not blobby enough, throw out
            float aspect = x_size / y_size;
            //std::cout << "Aspect: " << aspect << "\n";
            if (aspect <= 0.5 || aspect >= 2) {
#ifdef BLOBROI_DEBUG
                std::cout << "Aspect: " << aspect << "\n";
                std::cout << "Aspect bad\n";
#endif //BLOBROI_DEBUG
                continue;
            }

            // If the density of the pixels is low, throw out
            int area = (x_size * y_size);
            float density = (float) r.num_pixels / area;
            if (density <= 0.75) {
#ifdef BLOBROI_DEBUG
                std::cout << "Area: " << area << " Num pix:" << r.num_pixels << " density: " << density << "\n";
                std::cout << "Desity bad\n";
#endif //BLOBROI_DEBUG
                continue;
            }

            r.completely_internal = true;
            internal_region_features.num_internal_regions++;
            internal_region_features.num_regions++;

            // Find the most dence region and save its region number for use in BlobROI
            cur_den_err = 1 - density;
            if (cur_den_err < density_error) {
                density_error = cur_den_err;
                internal_region_features.max_region_density_number = internal_region_features.num_regions;
            }
#ifdef BLOBROI_DEBUG
            std::cout << "---Region Accepted---\n";
            std::cout << "Aspect: " << aspect << " Density: " << density << "\n";
#endif //BLOBROI_DEBUG
            internal_region_features.groups.push_back(r);
        }
    }
    internal_regions = internal_region_features;

#ifdef BALL_DEBUG
        std::cout << "Num Blob Regions: " << internal_region_features.num_regions << "\n";
#endif // BALL_DEBUG
}