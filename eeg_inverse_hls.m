function J_block = eeg_inverse_hls(Y_block, K, F)
    %#codegen
    
    % Y_block: [64 x block_size]
    % K:       [Ns x 64]
    % J_block: [Ns x block_size]
    % F fixed point arithmetic rules
    
    % Wide accumulator: Q8.40
    T_acc = numerictype(1, 48, 40);
    
    % Output
    T_out = T_acc;
    
    Ns = size(K,1);
    B  = size(Y_block, 2);
    N_elec = size(K,2);
    
    J_block = fi(zeros(Ns, B), T_out, F);
    
    for b = 1:B
        %#pragma HLS PIPELINE
        for s = 1:Ns
            %#pragma HLS PIPELINE
            if mod(s, 1000) == 0
                disp(s)
            end
            acc = fi(0, T_acc, F);
            for c = 1:N_elec
                %#pragma HLS UNROLL factor=8
                acc = acc + K(s,c) * Y_block(c,b);
            end
            J_block(s,b) = fi(acc, T_out, F);
        end
    end
end