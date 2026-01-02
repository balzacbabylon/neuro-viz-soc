# neuro-rtl
FPGA Hardware Acceleration of 3D reconstruction of EEG data

### What is being done, for whom, and why?

This project will aim to apply Hardware Acceleration to the algorithims responsible for the 3D-reconsttruction of EEG signals from the [BRAINSTORM](https://neuroimage.usc.edu/brainstorm/Introduction) Software ([GitHub repo](https://github.com/brainstorm-tools/brainstorm3)). 

This hopes to be a Proof of Concept for opening further research avenues into the topic of Closed-Loop Neuromodulation. State-Dependent Neuromodulation requires Real-time feedback of neural signal responses to stimulation, which an FPGA would enable. Initially, this 3D reconstruction could be used to visualize EEG responses in real time for research or clinical use. 

Ultimately, the idea however is to leverage the generated topography of 3D EEG activity for use as a structural "prior" to be then fed into Deep-Learning models for extrapolation or inferring connections based on eeg data. 

For the BRAINSTORM Project Itself, there have been discussion threads from users who are looking for real time processing options:
[Where do I find the option to connect Brainstorm to a live EEG amplifier? (2025)](https://neuroimage.usc.edu/forums/t/where-do-i-find-the-option-to-connect-brainstorm-to-a-live-eeg-amplifier/50239)
[Real Time Processing in Brain storm (2014)](https://neuroimage.usc.edu/forums/t/real-time-processing-in-brain-storm/1286)
The Resources Section below includes research regarding real-time processing for further reading

### What makes the project special and exciting?

Multimodal neuroimaging is a very exciting domain of research. Electroencephalography (EEG), Magnetoencephalography (MEG), and Functional Near-Infrared Spectroscopy (fNIRS) are noninvasive imaging techniques which all have very high temporal resolution (temporal = high sample rate, which can better be correlated with brain activity) but low spatial resolution (we can't pinpoint as precisely where the signals are coming from as compared to an MRI for example). Combining these imaging techniques, however, results in a fuller picture of what is going on in the brain. This takes the form of using post-processing software and toolboxes such as BRAINSTORM. This however is done _after the fact_.

The idea behind between accelerating computation of these data processing techniques is to open further possibilites for research and clinical domains, and allow for further vectors of analysis. Normally, the use of integrated circuits and FPGA's in neural processing occurs with invasive methodologies, but for noninvasive approaches this is still type of approach is still open for exploration. 

### How to get started?

The project is going to be implemented on a DE1-SoC development board. The tentative outline for what needs to be done is as follows:

Phase 1: Data Preparation (Host PC - Brainstorm)
Before touching the hardware, we need to generate "Golden Vectors" to verify hardware implementation later. The DE1-SoC has ~4.4 Mbits of on-chip block RAM (M10K). A Full Cortex Model consists of 15,000 sources×64 sensors×16bit≈15.3 Mbits, which won't fit in BRAM.

- [ ] For the PoC, generate a Scout-based (ROI) kernel. Using the Desikan-Killiany atlas (approx. 68 regions), the matrix becomes 68×64×16bit≈70 Kbits. This fits easily and leaves room for standard vertex-based localized patches to scale up later.
Steps:

- [ ] Generate Inverse: In Brainstorm, compute sources using wMNE.
- [ ] Extract Scouts: Use the "Scouts" tab to project sources to the ~68 anatomical regions (Select atlas -> Scout time series).
- [ ] Export Matrix: You need the ImagingKernel that maps Sensors → Scouts.
Note: Brainstorm usually computes Sources=K⋅Data, then averages sources into Scouts. For the FPGA, pre-multiply the Scout interpolation matrix with the Imaging Kernel to get a single 68×64 matrix.
- [ ] Export Data: Export a 10-second chunk of raw EEG data (64×Nsamples​) as eeg_stream.bin.
- [ ] Quantization: Convert both the Matrix and Data to Fixed Point (Q15.16) in Matlab/Python and save as binary files.

Phase 2: System Architecture (Qsys/Platform Designer)
Design a system where the HPS mimics the ADC and the FPGA performs the compute.

The Bus Architecture:
HPS-to-FPGA Lightweight AXI Bridge: Will be used for Control.
- The HPS uses this to write "Start/Stop" signals and update threshold parameters.
HPS-to-FPGA AXI Bridge (Heavyweight): Used for Data Streaming.
- Option A (Simple): Use a FIFO IP. HPS pushes sample t into the FIFO. FPGA pulls it.
On-Chip Memory (FPGA): Instantiate a Dual-Port RAM IP in Qsys to store your 68×64 Kernel.
Port A: Connected to HPS (to load the matrix at boot).
Port B: Connected to Custom Logic (to read during compute).

Phase 3:  EEG Data Stream (HPS Software - C/C++)


### resources

#ToDo: move to dedicated resources section

[FreeSurfer](https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferWiki)
- "a software package for the analysis and visualization of structural and functional neuroimaging data from cross-sectional or longitudinal studies"
- Has info on Desikan-Killiany atlas



Further Reading:
Papers:
rtMEG: A Real-Time Software Interface for Magnetoencephalography (2011)](https://onlinelibrary.wiley.com/doi/10.1155/2011/327953)
[Across-subject offline decoding of motor imagery from MEG and EEG (2018)](https://www.nature.com/articles/s41598-018-28295-z)
[A Study on Analysis Method for a Real-Time Neurofeedback System Using Non-Invasive Magnetoencephalography (2022](https://www.mdpi.com/2079-9292/11/15/2473)
[Requirement-driven model-based development methodology applied to the design of a real-time MEG data processing unit(2020)](https://duepublico2.uni-due.de/servlets/MCRFileNodeServlet/duepublico_derivate_00070153/Diss_Chen.pdf) <- implemented on FPGA
[FPGA implementation of 4-channel ICA for on-line EEG signal separation (2008)](https://ieeexplore.ieee.org/document/4696875)
[Development of a Low-Cost FPGA-Based SSVEP BCI Multimedia Control System (2010)](https://ieeexplore.ieee.org/abstract/document/5437509?casa_token=qybldk7BgvEAAAAA:oeEfawXhvOIfi9y7dUjurcm0iPZCKLVLnZ_c4ZkqlNh933kQvQUf0k2DRI-w6kSt9vZppLlK)
[Evaluation of artifact subspace reconstruction for automatic EEG artifact removal (2018) (400+ citations) ](https://ieeexplore.ieee.org/abstract/document/8512547?casa_token=0bW-djbKq94AAAAA:kPVrBjRBFtpeG1GuOOb93iTF6ykGH4grydu_HQChckw2YR8QWfYnQWVVVQPcxvc64GPlAX69)
[Beyond topographic mapping: Towards functional-anatomical imaging with 124-channel EEGs and 3-D MRIs (1990) (170+ citatations) (legacy)](https://link.springer.com/article/10.1007/BF01128862)
[Neuro-3D: Towards 3D Visual Decoding from EEG Signals (2025) (18 citations)](https://openaccess.thecvf.com/content/CVPR2025/papers/Guo_Neuro-3D_Towards_3D_Visual_Decoding_from_EEG_Signals_CVPR_2025_paper.pdf) 
[Inverse problem for M/EEG source localization: a review (2024) (7 citations)](https://ieeexplore.ieee.org/abstract/document/10768959?casa_token=hIIZd4ZvzGIAAAAA:nrIv566Q6BHkzCCEO40tQhvDMsv_U9ffSYnx64DLqPI2v-BA3_eIEYg0_SGir2g0NvU0R5GG)
[Computational models in electroencephalography (2022) (60+ citations)](https://link.springer.com/article/10.1007/s10548-021-00828-2)
[Improved localizadon of cortical activity by combining EEG and MEG with MRI cortical surface reconstruction: a linear approach (1993) (2600+ citations) (legacy)](https://direct.mit.edu/jocn/article-abstract/5/2/162/3095/Improved-Localizadon-of-Cortical-Activity-by)
[Human brain imaging with high-density electroencephalography: Techniques and applications (2024) (20+ citations)](https://physoc.onlinelibrary.wiley.com/doi/full/10.1113/JP286639)
[Texturing 3D-reconstructions of the human brain with EEG-activity maps (1998) (12 citations) (legacy)](https://onlinelibrary.wiley.com/doi/abs/10.1002/(SICI)1097-0193(1998)6:4%3C189::AID-HBM1%3E3.0.CO;2-%23?casa_token=74-Znk8jLmUAAAAA:lw9z9qc4pn1-7bf7rAmBmNaG6r5OhkOCNi16f55ZUNgDlihwBIXhKarIzpMoLwHDsITx8Y0LyHp97w)
- comment: the apprach in this paper seems fairly sophisticated. It would be a good exercise to identify what the limitations are and and how approaches differ today

[Review of methods for solving the EEG inverse problem (1999)(1200+ citations) (legacy)](https://www.uzh.ch/keyinst/NewLORETA/TechnicalDetails/TechnicalDetails.pdf)
- comment: Again, probably worth taking a look at to get a preliminary understanding of the inverse modeling problem
