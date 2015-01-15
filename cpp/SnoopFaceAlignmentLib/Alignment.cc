#include <SnoopFaceAlignmentLib/Alignment.h>
#include <eigen3/Eigen/Cholesky>


using namespace FACETRACKER;


const cv::Mat& LandmarkDetector::getReferenceShape() const
{
    return reference_shape_;
}


CSIROLandmarkDetector::CSIROLandmarkDetector(const std::string& tracker_model, const std::string& tracker_params)
{
    tracker_ = LoadFaceTracker(tracker_model.c_str());
    tracker_params_ = LoadFaceTrackerParams(tracker_params.c_str());

    reference_shape_ = cv::Mat(5, 2, CV_64FC1);
    reference_shape_.at<double>(0,0) = 125.0;
    reference_shape_.at<double>(0,1) = 140.0;
    reference_shape_.at<double>(1,0) = 125.0;
    reference_shape_.at<double>(1,1) = 90.0;
    reference_shape_.at<double>(2,0) = 100.0;
    reference_shape_.at<double>(2,1) = 110.0;
    reference_shape_.at<double>(3,0) = 150.0;
    reference_shape_.at<double>(3,1) = 110.0;
    reference_shape_.at<double>(4,0) = 125.0;
    reference_shape_.at<double>(4,1) = 175.0;
}

CSIROLandmarkDetector::~CSIROLandmarkDetector()
{
}

void CSIROLandmarkDetector::detectLandmarks(const cv::Mat& img, const cv::Rect& bounding_box, cv::Mat& landmarks) const
{
    cv::Mat non_const_img = img;
    
    tracker_->NewFrame(non_const_img, tracker_params_);
    PointVector detected_landmarks = tracker_->getShape();
    tracker_->Reset();
    
    int L = detected_landmarks.size();
    landmarks = cv::Mat(L, 2, CV_64FC1);
    for(int l=0; l<L; l++) {
        double* landmark = landmarks.ptr<double>(l);
        landmark[0] = detected_landmarks[l].x;
        landmark[1] = detected_landmarks[l].y;
    }
}

void CSIROLandmarkDetector::extractLandmarksForNormalization(const cv::Mat& landmarks, cv::Mat& normalization_landmarks) const
{
    normalization_landmarks = cv::Mat(5, 2, CV_64FC1);
    landmarks.row(30).copyTo(normalization_landmarks.row(0));
    normalization_landmarks.row(1) = (landmarks.row(19) + landmarks.row(24)) * 0.5;
    normalization_landmarks.row(2) = (landmarks.row(36) + landmarks.row(39)) * 0.5;
    normalization_landmarks.row(3) = (landmarks.row(42) + landmarks.row(45)) * 0.5;
    landmarks.row(64).copyTo(normalization_landmarks.row(4));
}

LBFLandmarkDetector::LBFLandmarkDetector()
{
}

LBFLandmarkDetector::~LBFLandmarkDetector()
{
    delete landmark_detector_;
}

void LBFLandmarkDetector::loadModel(const std::string& filename)
{
    landmark_detector_ = new LocalBinaryFeatureAlignment(filename);
    
    landmark_detector_->getMeanShape().copyTo(reference_shape_);
    landmark_detector_->adaptNormalizedShapeToFaceBoundingBox((double*) reference_shape_.data, cv::Rect(64, 64, 120, 120));
}

void LBFLandmarkDetector::detectLandmarks(const cv::Mat& img, const cv::Rect& bounding_box, cv::Mat& landmarks) const
{
    landmarks = landmark_detector_->align(bounding_box, img);
}

void LBFLandmarkDetector::extractLandmarksForNormalization(const cv::Mat& landmarks, cv::Mat& normalization_landmarks) const
{
    normalization_landmarks = landmarks;
}


void FaceNormalization::setReferenceShape(const cv::Mat& reference_shape)
{
    reference_shape_ = reference_shape;
    L_ = reference_shape.rows;
}

void FaceNormalization::normalize(cv::Mat& img, const cv::Mat& landmarks) const
{
    cv::Mat input_img;
    if(img.rows < 250 || img.cols < 250) {
        int input_img_rows = img.rows;
	int input_img_cols = img.cols;
        cv::Rect roi(0, 0, img.cols, img.rows);
        if(img.cols < 250) {
	    input_img_cols = 250;
	    roi.width = img.cols;
        }
	if(img.rows < 250) {
   	    input_img_rows = 250;
	    roi.height = img.rows;
	}
	input_img = cv::Mat::zeros(input_img_rows, input_img_cols, img.type());
	cv::Mat sub_img(input_img, roi);
	img.copyTo(sub_img);
	
	img = cv::Mat(input_img.rows, input_img.cols, input_img.type());
    }
    else
        img.copyTo(input_img);
    
    double transformation[6];
    computeSimilarityTransform((double*) landmarks.data, transformation);
    cv::warpAffine(input_img, img, cv::Mat(2, 3, CV_64FC1, transformation), img.size());
    
    if(img.rows != 250 || img.cols != 250) {
      cv::Mat cropped(img, cv::Rect(0, 0, 250, 250));
      cropped.copyTo(img);
    }
}

void FaceNormalization::computeSimilarityTransform(const double* src, double* transformation) const
{
    Eigen::Matrix<double, Eigen::Dynamic, 4> A(2*L_, 4);
    Eigen::VectorXd b(2*L_);
    
    for(int l=0; l<L_; l++) {
        A(2*l, 0) = src[2*l];
        A(2*l, 1) = -src[2*l+1];
        A(2*l, 2) = 1;
        A(2*l, 3) = 0;
        
        A(2*l+1, 0) = src[2*l+1];
        A(2*l+1, 1) = src[2*l];
        A(2*l+1, 2) = 0;
        A(2*l+1, 3) = 1;
        
        const double* reference_shape_landmark = reference_shape_.ptr<double>(l);
        b(2*l) = reference_shape_landmark[0];
        b(2*l+1) = reference_shape_landmark[1];
    }
    
    Eigen::Matrix<double, Eigen::Dynamic, 4> cov;
    cov.noalias() = A.transpose() * A;
    
    Eigen::Vector4d sol = cov.ldlt().solve(A.transpose() *b);
    
    transformation[0] = sol(0);
    transformation[1] = -sol(1);
    transformation[2] = sol(2);
    
    transformation[3] = sol(1);
    transformation[4] = sol(0);
    transformation[5] = sol(3);
}
