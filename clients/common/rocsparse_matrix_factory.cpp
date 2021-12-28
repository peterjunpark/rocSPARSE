/*! \file */
/* ************************************************************************
 * Copyright (c) 2021 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ************************************************************************ */

#include "rocsparse_matrix_factory.hpp"
#include "rocsparse_init.hpp"
#include "rocsparse_matrix_utils.hpp"

template <typename T, typename I, typename J>
rocsparse_matrix_factory<T, I, J>::~rocsparse_matrix_factory()
{
    if(this->m_instance)
    {
        delete this->m_instance;
        this->m_instance = nullptr;
    }
}

template <typename T, typename I, typename J>
rocsparse_matrix_factory<T, I, J>::rocsparse_matrix_factory(const Arguments&      arg,
                                                            rocsparse_matrix_init matrix,
                                                            bool                  to_int, // = false
                                                            bool full_rank, // = false
                                                            bool noseed // = false
                                                            )
    : m_arg(arg)
{
    //
    // FORCE REINIT.
    //
    if(false == noseed)
    {
        rocsparse_seedrand();
    }

    switch(matrix)
    {
    case rocsparse_matrix_random:
    {
        rocsparse_matrix_init_kind matrix_init_kind = arg.matrix_init_kind;
        this->m_instance
            = new rocsparse_matrix_factory_random<T, I, J>(full_rank, to_int, matrix_init_kind);
        break;
    }

    case rocsparse_matrix_laplace_2d:
    {
        this->m_instance = new rocsparse_matrix_factory_laplace2d<T, I, J>(arg.dimx, arg.dimy);
        break;
    }

    case rocsparse_matrix_laplace_3d:
    {
        this->m_instance
            = new rocsparse_matrix_factory_laplace3d<T, I, J>(arg.dimx, arg.dimy, arg.dimz);
        break;
    }

    case rocsparse_matrix_file_rocalution:
    {
        std::string filename = arg.timing
                                   ? arg.filename
                                   : rocsparse_exepath() + "../matrices/" + arg.filename + ".csr";
        this->m_instance
            = new rocsparse_matrix_factory_rocalution<T, I, J>(filename.c_str(), to_int);
        break;
    }

    case rocsparse_matrix_file_rocsparseio:
    {
        std::string filename = arg.timing
                                   ? arg.filename
                                   : rocsparse_exepath() + "../matrices/" + arg.filename + ".bin";
        this->m_instance
            = new rocsparse_matrix_factory_rocsparseio<T, I, J>(filename.c_str(), to_int);
        break;
    }

    case rocsparse_matrix_file_mtx:
    {
        std::string filename = arg.timing
                                   ? arg.filename
                                   : rocsparse_exepath() + "../matrices/" + arg.filename + ".mtx";
        this->m_instance     = new rocsparse_matrix_factory_mtx<T, I, J>(filename.c_str());
        break;
    }

    case rocsparse_matrix_zero:
    {
        this->m_instance = new rocsparse_matrix_factory_zero<T, I, J>();
        break;
    }

    default:
    {
        this->m_instance = nullptr;
        break;
    }
    }
    assert(this->m_instance != nullptr);
}

template <typename T, typename I, typename J>
rocsparse_matrix_factory<T, I, J>::rocsparse_matrix_factory(const Arguments& arg,
                                                            bool             to_int, //  = false,
                                                            bool             full_rank, // = false,
                                                            bool             noseed) //  = false)
    : rocsparse_matrix_factory(arg, arg.matrix, to_int, full_rank, noseed)
{
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_csr(
    std::vector<I>&       csr_row_ptr,
    std::vector<J>&       csr_col_ind,
    std::vector<T>&       csr_val,
    J&                    M,
    J&                    N,
    I&                    nnz,
    rocsparse_index_base  base,
    rocsparse_matrix_type matrix_type, // = rocsparse_matrix_type_general,
    rocsparse_fill_mode   uplo) // = rocsparse_fill_mode_lower)
{
    this->m_instance->init_csr(
        csr_row_ptr, csr_col_ind, csr_val, M, N, nnz, base, matrix_type, uplo);
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_gebsr(std::vector<I>&      bsr_row_ptr,
                                                   std::vector<J>&      bsr_col_ind,
                                                   std::vector<T>&      bsr_val,
                                                   rocsparse_direction  dirb,
                                                   J&                   Mb,
                                                   J&                   Nb,
                                                   I&                   nnzb,
                                                   J&                   row_block_dim,
                                                   J&                   col_block_dim,
                                                   rocsparse_index_base base)
{
    this->m_instance->init_gebsr(
        bsr_row_ptr, bsr_col_ind, bsr_val, dirb, Mb, Nb, nnzb, row_block_dim, col_block_dim, base);
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_bsr(std::vector<I>&      bsr_row_ptr,
                                                 std::vector<J>&      bsr_col_ind,
                                                 std::vector<T>&      bsr_val,
                                                 rocsparse_direction  dirb,
                                                 J&                   Mb,
                                                 J&                   Nb,
                                                 I&                   nnzb,
                                                 J&                   block_dim,
                                                 rocsparse_index_base base)
{
    this->m_instance->init_gebsr(
        bsr_row_ptr, bsr_col_ind, bsr_val, dirb, Mb, Nb, nnzb, block_dim, block_dim, base);
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_coo(std::vector<I>&      coo_row_ind,
                                                 std::vector<I>&      coo_col_ind,
                                                 std::vector<T>&      coo_val,
                                                 I&                   M,
                                                 I&                   N,
                                                 I&                   nnz,
                                                 rocsparse_index_base base)
{
    this->m_instance->init_coo(coo_row_ind, coo_col_ind, coo_val, M, N, nnz, base);
}

// @brief Init host csr matrix.

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_csr(
    host_csr_matrix<T, I, J>& that,
    J&                        m,
    J&                        n,
    rocsparse_index_base      base,
    rocsparse_matrix_type     matrix_type, // = rocsparse_matrix_type_general,
    rocsparse_fill_mode       uplo) //      = rocsparse_fill_mode_lower)
{
    that.base = base;
    that.m    = m;
    that.n    = n;
    this->m_instance->init_csr(
        that.ptr, that.ind, that.val, that.m, that.n, that.nnz, that.base, matrix_type, uplo);
    m = that.m;
    n = that.n;
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_csc(host_csc_matrix<T, I, J>& that,
                                                 J&                        m,
                                                 J&                        n,
                                                 rocsparse_index_base      base)
{
    that.base = base;
    this->m_instance->init_csr(that.ptr, that.ind, that.val, n, m, that.nnz, that.base);
    that.m = m;
    that.n = n;
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_csr(host_csr_matrix<T, I, J>& that)
{
    that.base = this->m_arg.baseA;
    that.m    = this->m_arg.M;
    that.n    = this->m_arg.N;
    this->m_instance->init_csr(that.ptr,
                               that.ind,
                               that.val,
                               that.m,
                               that.n,
                               that.nnz,
                               that.base,
                               this->m_arg.matrix_type,
                               this->m_arg.uplo);
}

template <typename T, typename I, typename J, class FILTER = void>
struct traits_init_bsr
{

    static void init(rocsparse_matrix_factory<T, I, J>& factory,
                     host_gebsr_matrix<T, I, J>&        that,
                     device_gebsr_matrix<T, I, J>&      that_on_device,
                     J&                                 mb_,
                     J&                                 nb_)
    {
        std::cerr << "ICITE2 " << std::endl;
        exit(1);
    };
};

template <typename T, typename I, typename J>
struct traits_init_bsr<
    T,
    I,
    J,
    std::enable_if_t<std::is_same<rocsparse_int, J>{} && std::is_same<rocsparse_int, I>{}>>
{

    static void init(rocsparse_matrix_factory<T, I, J>& factory,
                     host_gebsr_matrix<T, I, J>&        that,
                     device_gebsr_matrix<T, I, J>&      that_on_device,
                     J&                                 mb_,
                     J&                                 nb_)
    {
        //
        // Initialize in case init_csr requires it as input.
        //
        rocsparse_int            block_dim = factory.m_arg.block_dim;
        rocsparse_int            M         = mb_ * block_dim;
        rocsparse_int            N         = nb_ * block_dim;
        rocsparse_index_base     base      = factory.m_arg.baseA;
        host_csr_matrix<T, I, J> hA_uncompressed;
        factory.init_csr(hA_uncompressed, M, N, base);

        {
            device_csr_matrix<T, I, J> dA_uncompressed(hA_uncompressed);
            device_csr_matrix<T, I, J> dA_compressed;
            rocsparse_matrix_utils::compress(dA_compressed, dA_uncompressed, factory.m_arg.baseA);
            rocsparse_matrix_utils::convert(
                dA_compressed, factory.m_arg.direction, block_dim, base, that_on_device);
        }

        that(that_on_device);

        mb_ = that.mb;
        nb_ = that.nb;
    };
};

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_bsr(host_gebsr_matrix<T, I, J>& that, J& mb_, J& nb_)
{
    device_gebsr_matrix<T, I, J> dB;
    this->init_bsr(that, dB, mb_, nb_);
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_bsr(host_gebsr_matrix<T, I, J>&   that,
                                                 device_gebsr_matrix<T, I, J>& that_on_device,
                                                 J&                            mb_,
                                                 J&                            nb_)
{
    traits_init_bsr<T, I, J>::init(*this, that, that_on_device, mb_, nb_);
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_gebsr(host_gebsr_matrix<T, I, J>& that,
                                                   rocsparse_direction         block_dir_,
                                                   J&                          mb_,
                                                   J&                          nb_,
                                                   I&                          nnzb_,
                                                   J&                          row_block_dim_,
                                                   J&                          col_block_dim_,
                                                   rocsparse_index_base        base_)
{
    that.block_direction = block_dir_;
    that.mb              = mb_;
    that.nb              = nb_;
    that.row_block_dim   = row_block_dim_;
    that.col_block_dim   = col_block_dim_;
    that.base            = base_;
    that.nnzb            = nnzb_;
    this->m_instance->init_gebsr(that.ptr,
                                 that.ind,
                                 that.val,
                                 that.block_direction,
                                 that.mb,
                                 that.nb,
                                 that.nnzb,
                                 that.row_block_dim,
                                 that.col_block_dim,
                                 that.base);

    mb_            = that.mb;
    nb_            = that.nb;
    nnzb_          = that.nnzb;
    row_block_dim_ = that.row_block_dim;
    col_block_dim_ = that.col_block_dim;
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_gebsr(host_gebsr_matrix<T, I, J>& that)
{
    that.block_direction = this->m_arg.direction;
    that.mb              = this->m_arg.M;
    that.nb              = this->m_arg.N;
    that.nnzb            = this->m_arg.nnz;
    that.row_block_dim   = this->m_arg.row_block_dimA;
    that.col_block_dim   = this->m_arg.col_block_dimA;
    that.base            = this->m_arg.baseA;
    this->m_instance->init_gebsr(that.ptr,
                                 that.ind,
                                 that.val,
                                 that.block_direction,
                                 that.mb,
                                 that.nb,
                                 that.nnzb,
                                 that.row_block_dim,
                                 that.col_block_dim,
                                 that.base);
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_gebsr(
    host_gebsr_matrix<T, I, J>& that, J& mb, J& nb, J& row_block_dim, J& col_block_dim)
{

    that.base          = this->m_arg.baseA;
    that.mb            = mb;
    that.nb            = nb;
    that.nnzb          = this->m_arg.nnz;
    that.row_block_dim = row_block_dim;
    that.col_block_dim = col_block_dim;

    this->m_instance->init_gebsr(that.ptr,
                                 that.ind,
                                 that.val,
                                 that.block_direction,
                                 that.mb,
                                 that.nb,
                                 that.nnzb,
                                 that.row_block_dim,
                                 that.col_block_dim,
                                 that.base);

    mb            = that.mb;
    nb            = that.nb;
    row_block_dim = that.row_block_dim;
    col_block_dim = that.col_block_dim;
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_csr(host_csr_matrix<T, I, J>& that, J& m, J& n)
{
    this->init_csr(that, m, n, this->m_arg.baseA, this->m_arg.matrix_type, this->m_arg.uplo);
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_gebsr_spezial(host_gebsr_matrix<T, I, J>& that,
                                                           J&                          Mb,
                                                           J&                          Nb)
{
    I idx = 0;

    host_csr_matrix<T, I, J> hA;
    rocsparse_direction      direction     = this->m_arg.direction;
    rocsparse_index_base     base          = this->m_arg.baseA;
    J                        row_block_dim = this->m_arg.row_block_dimA;
    J                        col_block_dim = this->m_arg.col_block_dimA;
    this->init_csr(hA, Mb, Nb, base);

    that.define(direction, Mb, Nb, hA.nnz, row_block_dim, col_block_dim, base);

    switch(direction)
    {
    case rocsparse_direction_column:
    {
        T*       val    = that.val;
        const I* hA_ptr = hA.ptr.data();
        for(J i = 0; i < Mb; ++i)
        {
            for(J r = 0; r < row_block_dim; ++r)
            {
                for(I k = hA_ptr[i] - base; k < hA_ptr[i + 1] - base; ++k)
                {
                    for(J c = 0; c < col_block_dim; ++c)
                    {
                        val[k * row_block_dim * col_block_dim + c * row_block_dim + r]
                            = static_cast<T>(++idx);
                    }
                }
            }
        }
        break;
    }

    case rocsparse_direction_row:
    {
        T*       val    = that.val;
        const I* hA_ptr = hA.ptr.data();
        for(J i = 0; i < Mb; ++i)
        {
            for(J r = 0; r < row_block_dim; ++r)
            {
                for(I k = hA_ptr[i] - base; k < hA_ptr[i + 1] - base; ++k)
                {
                    for(J c = 0; c < col_block_dim; ++c)
                    {
                        val[k * row_block_dim * col_block_dim + r * col_block_dim + c]
                            = static_cast<T>(++idx);
                    }
                }
            }
        }
        break;
    }
    }

    that.ptr.transfer_from(hA.ptr);
    that.ind.transfer_from(hA.ind);
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_coo(host_coo_matrix<T, I>& that)
{
    that.base = this->m_arg.baseA;
    that.m    = this->m_arg.M;
    that.n    = this->m_arg.N;
    this->m_instance->init_coo(
        that.row_ind, that.col_ind, that.val, that.m, that.n, that.nnz, that.base);
}

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_coo(
    host_coo_matrix<T, I>& that,
    I&                     M,
    I&                     N,
    rocsparse_index_base   base,
    rocsparse_matrix_type  matrix_type, // = rocsparse_matrix_type_general,
    rocsparse_fill_mode    uplo) //      = rocsparse_fill_mode_lower)
{
    that.base = base;
    that.m    = M;
    that.n    = N;
    this->m_instance->init_coo(
        that.row_ind, that.col_ind, that.val, that.m, that.n, that.nnz, that.base);
    M = that.m;
    N = that.n;
}

template <typename T, typename I, typename J, class FILTER = void>
struct traits_init_coo_aos
{

    static void init(rocsparse_matrix_factory<T, I, J>& factory,
                     host_coo_aos_matrix<T, I>&         that,
                     I&                                 M,
                     I&                                 N,
                     rocsparse_index_base               base,
                     rocsparse_matrix_type matrix_type, // = rocsparse_matrix_type_general,
                     rocsparse_fill_mode   uplo) //   = rocsparse_fill_mode_lower)
    {
        std::cerr << "non reachable " << __LINE__ << std::endl;
        exit(1);
    };
};

template <typename T, typename I, typename J>
struct traits_init_coo_aos<T, I, J, std::enable_if_t<std::is_same<I, J>{}>>
{

    static void init(rocsparse_matrix_factory<T, I, J>& factory,
                     host_coo_aos_matrix<T, I>&         that,
                     I&                                 M,
                     I&                                 N,
                     rocsparse_index_base               base,
                     rocsparse_matrix_type matrix_type, // = rocsparse_matrix_type_general,
                     rocsparse_fill_mode   uplo) //   = rocsparse_fill_mode_lower)
    {
        host_csr_matrix<T, I, I> hA;
        factory.init_csr(hA, M, N, base, matrix_type, uplo);
        that.define(hA.m, hA.n, hA.nnz, hA.base);
        host_csr_to_coo_aos(hA.m, hA.nnz, hA.ptr, hA.ind, that.ind, hA.base);
        that.val.transfer_from(hA.val);
    };
};

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_coo_aos(
    host_coo_aos_matrix<T, I>& that,
    I&                         M,
    I&                         N,
    rocsparse_index_base       base,
    rocsparse_matrix_type      matrix_type, // = rocsparse_matrix_type_general,
    rocsparse_fill_mode        uplo) //   = rocsparse_fill_mode_lower)
{
    traits_init_coo_aos<T, I, J>::init(*this, that, M, N, base, matrix_type, uplo);
}

template <typename T, typename I, typename J, class FILTER = void>
struct traits_init_ell
{
    static void init(rocsparse_matrix_factory<T, I, J>& factory,
                     host_ell_matrix<T, I>&             that,
                     I&                                 M,
                     I&                                 N,
                     rocsparse_index_base               base,
                     rocsparse_matrix_type matrix_type, // = rocsparse_matrix_type_general,
                     rocsparse_fill_mode   uplo) //   = rocsparse_fill_mode_lower)
    {
        std::cerr << "non reachable " << __LINE__ << std::endl;
        exit(1);
    };
};

template <typename T, typename I, typename J>
struct traits_init_ell<T, I, J, std::enable_if_t<std::is_same<I, J>{}>>
{
    static void init(rocsparse_matrix_factory<T, I, J>& factory,
                     host_ell_matrix<T, I>&             that,
                     I&                                 M,
                     I&                                 N,
                     rocsparse_index_base               base,
                     rocsparse_matrix_type matrix_type, // = rocsparse_matrix_type_general,
                     rocsparse_fill_mode   uplo) //   = rocsparse_fill_mode_lower)
    {
        host_csr_matrix<T, I, I> hA;
        factory.init_csr(hA, M, N, base, matrix_type, uplo);
        that.define(hA.m, hA.n, 0, hA.base);
        host_csr_to_ell(
            hA.m, hA.ptr, hA.ind, hA.val, that.ind, that.val, that.width, hA.base, that.base);
        that.nnz = that.width * that.m;
    };
};

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_ell(
    host_ell_matrix<T, I>& that,
    I&                     M,
    I&                     N,
    rocsparse_index_base   base,
    rocsparse_matrix_type  matrix_type, // = rocsparse_matrix_type_general,
    rocsparse_fill_mode    uplo) // = rocsparse_fill_mode_lower)
{
    traits_init_ell<T, I, J>::init(*this, that, M, N, base, matrix_type, uplo);
}

template <typename T, typename I, typename J, class FILTER = void>
struct traits_init_hyb
{
    static void init(rocsparse_matrix_factory<T, I, J>& factory,
                     rocsparse_hyb_mat                  that,
                     I&                                 M,
                     I&                                 N,
                     I&                                 nnz,
                     rocsparse_index_base               base,
                     bool&                              conform)
    {
        std::cerr << "non reachable " << __LINE__ << std::endl;
        exit(1);
    };
};

template <typename T, typename I, typename J>
struct traits_init_hyb<T,
                       I,
                       J,
                       std::enable_if_t<std::is_same<I, J>{} && std::is_same<rocsparse_int, J>{}>>
{
    static void init(rocsparse_matrix_factory<T, I, J>& factory,
                     rocsparse_hyb_mat                  that,
                     I&                                 M,
                     I&                                 N,
                     I&                                 nnz,
                     rocsparse_index_base               base,
                     bool&                              conform)
    {
        conform                                = true;
        rocsparse_hyb_partition part           = factory.m_arg.part;
        rocsparse_int           user_ell_width = factory.m_arg.algo;

        host_csr_matrix<T, I, I> hA;
        factory.init_csr(hA, M, N, base);
        nnz = hA.nnz;

        // ELL width limit
        rocsparse_int width_limit = 2 * (hA.nnz - 1) / M + 1;

        // Limit ELL user width
        if(part == rocsparse_hyb_partition_user)
        {
            user_ell_width *= (hA.nnz / M);
            user_ell_width = std::min(width_limit, user_ell_width);
        }

        if(part == rocsparse_hyb_partition_max)
        {
            // Compute max ELL width
            rocsparse_int ell_max_width = 0;
            for(rocsparse_int i = 0; i < M; ++i)
            {
                ell_max_width = std::max(hA.ptr[i + 1] - hA.ptr[i], ell_max_width);
            }

            if(ell_max_width > width_limit)
            {
                conform = false;
                return;
            }
        }

        device_csr_matrix<T, I, I> dA(hA);

        rocsparse_handle handle;
        CHECK_ROCSPARSE_ERROR(rocsparse_create_handle(&handle));
        rocsparse_mat_descr descr;
        CHECK_ROCSPARSE_ERROR(rocsparse_create_mat_descr(&descr));
        // Set matrix index base
        CHECK_ROCSPARSE_ERROR(rocsparse_set_mat_index_base(descr, base));

        // Convert CSR matrix to HYB
        CHECK_ROCSPARSE_ERROR(rocsparse_csr2hyb<T>(
            handle, M, N, descr, dA.val, dA.ptr, dA.ind, that, user_ell_width, part));

        CHECK_ROCSPARSE_ERROR(rocsparse_destroy_mat_descr(descr));
        CHECK_ROCSPARSE_ERROR(rocsparse_destroy_handle(handle));
    };
};

template <typename T, typename I, typename J>
void rocsparse_matrix_factory<T, I, J>::init_hyb(
    rocsparse_hyb_mat that, I& M, I& N, I& nnz, rocsparse_index_base base, bool& conform)
{
    traits_init_hyb<T, I, J>::init(*this, that, M, N, nnz, base, conform);
}

template struct rocsparse_matrix_factory<float, int32_t, int32_t>;
template struct rocsparse_matrix_factory<float, int64_t, int32_t>;
template struct rocsparse_matrix_factory<float, int64_t, int64_t>;

template struct rocsparse_matrix_factory<double, int32_t, int32_t>;
template struct rocsparse_matrix_factory<double, int64_t, int32_t>;
template struct rocsparse_matrix_factory<double, int64_t, int64_t>;

template struct rocsparse_matrix_factory<rocsparse_float_complex, int32_t, int32_t>;
template struct rocsparse_matrix_factory<rocsparse_float_complex, int64_t, int32_t>;
template struct rocsparse_matrix_factory<rocsparse_float_complex, int64_t, int64_t>;

template struct rocsparse_matrix_factory<rocsparse_double_complex, int32_t, int32_t>;
template struct rocsparse_matrix_factory<rocsparse_double_complex, int64_t, int32_t>;
template struct rocsparse_matrix_factory<rocsparse_double_complex, int64_t, int64_t>;
