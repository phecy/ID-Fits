import os
import argparse
import subprocess

execfile(os.path.join(os.path.dirname(__file__), "fix_imports.py"))
import config



if __name__ == "__main__":

    # Parse commandline arguments
    parser = argparse.ArgumentParser(description="Generate models and descriptors for descriptors analysis.")
    parser.add_argument("dataset", nargs="?", default="lfwa", help="Dataset to use")
    args = parser.parse_args()
    

    # Variables
    compute_descriptor_script = os.path.join(config.bin_path, "generate_benchmarks_descriptors.py")
    learn_model_script = os.path.join(config.bin_path, "learn_benchmarks_models.py")
    dataset = args.dataset
    dataset_file = os.path.join(config.lfw_path, "%s.npy" % dataset)
    training_descriptors_path = os.path.join(config.lfw_benchmark_path, "training")


    # Train models
    print "\n\n------------- Training models -------------\n\n"
    subprocess.call(["python2", compute_descriptor_script, "lfw" ,"training", "ulbp", dataset_file])
    subprocess.call(["python2", learn_model_script, "lfw", os.path.join(training_descriptors_path, "ulbp_not_normalized_%s" % dataset), "pca", "200"])
    subprocess.call(["python2", compute_descriptor_script, "lfw" ,"training", "ulbp_pca", dataset_file])
    subprocess.call(["python2", learn_model_script, "lfw", os.path.join(training_descriptors_path, "ulbp_pca_not_normalized_%s" % dataset), "lda", "50"])
    subprocess.call(["python2", learn_model_script, "lfw", os.path.join(training_descriptors_path, "ulbp_pca_not_normalized_%s" % dataset), "jb", "50"])


    # Compute test descriptors
    print "\n\n------------- Computing test descriptors -------------\n\n"
    subprocess.call(["python2", compute_descriptor_script, "lfw" ,"test", "-n", "lbp", dataset_file])
    subprocess.call(["python2", compute_descriptor_script, "lfw" ,"test", "-n", "ulbp", dataset_file])
    subprocess.call(["python2", compute_descriptor_script, "lfw" ,"test", "-n", "ulbp_pca", dataset_file])
    subprocess.call(["python2", compute_descriptor_script, "lfw" ,"test", "-n", "ulbp_wpca", dataset_file])
    subprocess.call(["python2", compute_descriptor_script, "lfw" ,"test", "-n", "ulbp_pca_lda", dataset_file])
    subprocess.call(["python2", compute_descriptor_script, "lfw" ,"test", "ulbp_pca_jb", dataset_file])
