#include <stdlib.h>
#include <stdio.h>
#include <liquid/liquid.h>

void testfft();
void testfirfilter();

void testfft() {
	unsigned int n = 8;
	int flags = 0;
	int samplingfreq = 10000;

	float complex * x = (float complex*) malloc(n * sizeof(float complex));
    float complex * y = (float complex*) malloc(n * sizeof(float complex));

    // create fft plan
    fftplan q = fft_create_plan(n, x, y, LIQUID_FFT_FORWARD, flags);

    //int inp[100] = {159,484,160,475,179,447,230,386,284,328,359,255,413,206,464,166,483,158,476,178,447,216,477,480,440,368,284,209,165,163,204,276,361,435,478,479,436,364,279,206,164,164,207,281,365,438,480,478,434,360,275,203,159,165,210,285,369,441,480,477,431,355,270,200,162,167,213,289,374,444,481,475,427,351,266,196,161,168,216,294,378,447,482,473,423,346,262,194,160,170,220,298,382,450,483,472,420,342,258,191};
    int inp[8] = {1, 0, 0, 0, 0, 0, 0, 0};

    // init input
    for(int i = 0; i < n; i++) {
    	//x[i] = i % 2 == 0 ? 1.0 : -1.0;
        x[i] = (float complex) inp[i];
    	y[i] = 0.0;
    	//printf("%f%+fi ", crealf(x[i]), cimagf(x[i]));
    }
    printf("\n");

    fft_execute(q);

    for(int i = 0; i < n; i++) {
        // crealf(num) is real part, cimagf(num) is imaginary part
        // cabsf(num) is magnitude
        float complex cur = y[i];
    	//if(crealf(cur) >= 0.0) 
    	printf("%f+%fi at magnitude:%f at %fHz\n", crealf(cur), cimagf(cur), cabsf(cur), ((float)i/n)*samplingfreq);
    }
    printf("\n");

    fft_destroy_plan(q);
    free(x);
    free(y);
}

void testfirfilter() {
	// options
    float fc=0.15f;         // filter cutoff frequency
    float ft=0.05f;         // filter transition
    float As=60.0f;         // stop-band attenuation [dB]
    float mu=0.0f;          // fractional timing offset

    // estimate required filter length and generate filter
    unsigned int h_len = estimate_req_filter_len(ft,As);
    float h[h_len];
    liquid_firdes_kaiser(h_len,fc,As,mu,h);
}

int main() {
	testfft();
}