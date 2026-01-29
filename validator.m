function validator()

    % Fixed point arithmetic stuff
    % Q8.40: signed
    T_in = numerictype(1, 48, 40);
    
    % Define arithmetic rules
    % Large for accumulator computations
    F = fimath( ...
        'RoundingMethod', 'Nearest', ...
        'OverflowAction', 'Saturate', ...
        'ProductMode', 'SpecifyPrecision', ...
        'ProductWordLength', 64, ...
        'ProductFractionLength', 48, ...
        'SumMode', 'SpecifyPrecision', ...
        'SumWordLength', 48, ...
        'SumFractionLength', 40, 'CastBeforeSum', false);

    eeg_file = "/MATLAB Drive/neuro-rtl/data/EEG/data_SPIKE_average_260128_1703.mat";
    imaging_kernel_file = "/MATLAB Drive/neuro-rtl/data/EEG/results_sLORETA_EEG_KERNEL_260128_1749.mat";
    
    loaded_eeg = load(eeg_file);
    R = load(imaging_kernel_file);
    Y_float = loaded_eeg.F(R.GoodChannel, :);
    K_float = R.ImagingKernel;

    block_size = 1;
    
    block = 1: block_size;
    Y_block_float = Y_float(:, block);

    Y_block_fixed = fi(Y_block_float, 'numerictype', T_in, 'fimath', F);
    K_fixed       = fi(K_float,       'numerictype', T_in, 'fimath', F);

    J_float = K_float * Y_block_float;
    J_fixed = eeg_inverse_hls(Y_block_fixed, K_fixed, F);
    
    rms_err = rms(double(J_fixed(:)) - J_float(:));
    rms_ref = rms(J_float(:));
    rms_err / rms_ref

end