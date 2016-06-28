#include "HDFDataStore.h"

HDFDataStore::HDFDataStore(const std::string& filename, const bool writable)
{
    auto mode = writable ? H5F_ACC_TRUNC : H5F_ACC_RDONLY;
    file = H5::H5File(filename, mode);

    gp = file.createGroup(groupName);
}

void HDFDataStore::writeEvent(const Event& evt)
{
    const arma::uword nTraces = evt.numTraces();
    const arma::uword nColumns = 512 + 5;  // (cobo/asad/aget/ch/pad) + number of TBs
    arma::Mat<sample_t> dataMat (nTraces, nColumns);

    arma::uword rowNumber = 0;
    for (auto it = evt.cbegin(); it != evt.cend(); it++) {
        const HardwareAddress& addr = it->first;
        const arma::Col<sample_t>& tr = it->second;

        dataMat.row(rowNumber) = tr.t();

        rowNumber++;
    }

    arma::inplace_trans(dataMat);  // HACK: arma is col-major, hdf5 is row-major

    const int dspace_rank = 2;
    const hsize_t dspace_dims[2] = {nTraces, nColumns};
    H5::DataSpace dspace (dspace_rank, dspace_dims);

    const std::string dset_name = std::to_string(evt.eventId);

    H5::DataSet dset = gp.createDataSet(dset_name, H5::PredType::NATIVE_INT16, dspace);
    dset.write(dataMat.memptr(), H5::PredType::NATIVE_INT16);
}
