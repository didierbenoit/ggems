// GGEMS Copyright (C) 2017

/*!
 * \file linac_source.cu
 * \brief Linac source
 * \author Julien Bert <bert.jul@gmail.com>
 * \version 0.2
 * \date Thursday September 1st, 2016
 *
 * v0.2: JB - Change all structs and remove CPU exec
 *
*/

#ifndef LINAC_SOURCE_CU
#define LINAC_SOURCE_CU

#include "linac_source.cuh"

///////// GPU code ////////////////////////////////////////////////////

// Internal function that create a new particle to the buffer at the slot id
__host__ __device__ void linac_source ( ParticlesData *particles,
                                        const LinacSourceData *linac,
                                        f32matrix44 trans, ui32 id )
{
    // Main vars
    ui32 offset, ind;
    f32 rnd, xa, ya, xb, yb;

    // 1. Get position (rho)

    rnd = prng_uniform( particles, id );
    ind = binary_search_index( rnd, linac->cdf_rho, 0, (linac->n_rho-1) );

    f32 rho;
    if ( ind == 0 )
    {
        rho = 0.0f;
    }
    else
    {
        xa = linac->cdf_rho[ ind - 1 ];
        xb = linac->cdf_rho[ ind];
        ya = f32(ind - 1) * linac->s_rho;
        yb = f32(ind) * linac->s_rho;
        rho = linear_interpolation( xa, ya, xb, yb, rnd );
    }

    // 2. Get 2D position

    f32 psi = prng_uniform( particles, id ) * twopi;
    f32xyz pos = { 0.0, 0.0, 0.0 };    
    pos.x = rho * cos( psi );
    pos.y = rho * sin( psi );
    pos = fxyz_local_to_global_position( trans, pos );
    particles->px[ id ] = pos.x;                        // Position in mm
    particles->py[ id ] = pos.y;                        //
    particles->pz[ id ] = pos.z;                        //

    // 2. Get energy (rho E)

    ui32 irho = floor( f64(rho) / f64(linac->s_rho_E.x) );

#ifdef DEBUG
    assert( irho < linac->n_rho_E.x );
#endif

    rnd = prng_uniform( particles, id );
    offset = irho * linac->n_rho_E.y;
    ind = binary_search_index( rnd, linac->cdf_rho_E, offset, offset+linac->n_rho_E.y-1 );

    f32 E;
    if ( ind == 0 )
    {
        E = 0.0f;   // WARNING Should be Emin ?! - JB
    }
    else
    {
        xa = linac->cdf_rho_E[ ind - 1 ];
        xb = linac->cdf_rho_E[ ind ];
        ya = f32( ind-offset - 1 ) * linac->s_rho_E.y;
        yb = f32( ind-offset ) * linac->s_rho_E.y;
        E = linear_interpolation( xa, ya, xb, yb, rnd );
    }

    particles->E[ id ] = E;

    // 3. Get direction

    // 3.1 Get theta (rho E Theta)

    irho = floor( f64(rho) / f64(linac->s_rho_E_theta.x) );

#ifdef DEBUG
    assert( irho < linac->n_rho_E_theta.x );
#endif

    ui32 iE = floor( f64(E) / f64(linac->s_rho_E_theta.y) );

#ifdef DEBUG
    assert( iE < linac->n_rho_E_theta.y );
#endif

    f32 theta_max = linac->val_rho_E_theta_max[ irho*linac->n_rho_E_theta.y + iE ];
    f32 theta_spacing = theta_max / f32( linac->n_rho_E_theta.z );

    rnd = prng_uniform( particles, id );
    offset = irho*linac->n_rho_E_theta.y*linac->n_rho_E_theta.z + iE*linac->n_rho_E_theta.z;
    ind = binary_search_index( rnd, linac->cdf_rho_E_theta, offset, offset+linac->n_rho_E_theta.z-1 );

    f32 theta;
    if ( ind == 0 )
    {
        theta = 0.0f;
    }
    else
    {
        xa = linac->cdf_rho_E_theta[ ind - 1 ];
        xb = linac->cdf_rho_E_theta[ ind ];
        ya = f32( ind-offset - 1 ) * theta_spacing;
        yb = f32( ind-offset ) * theta_spacing;
        theta = linear_interpolation( xa, ya, xb, yb, rnd );
    }   

    // 3.2 Get phi

    irho = floor( f64(rho) / f64(linac->s_rho_theta_phi.x) );

#ifdef DEBUG
    assert( irho < linac->n_rho_theta_phi.x );
#endif

    ui32 itheta = floor( f64(theta) / f64(linac->s_rho_theta_phi.y) );

#ifdef DEBUG
    assert( itheta < linac->n_rho_theta_phi.y );
#endif

    rnd = prng_uniform( particles, id );
    offset = irho*linac->n_rho_theta_phi.y*linac->n_rho_theta_phi.z + itheta*linac->n_rho_theta_phi.z;
    ind = binary_search_index( rnd, linac->cdf_rho_theta_phi, offset, offset+linac->n_rho_theta_phi.z-1 );

    f32 phi;
    if ( ind == 0 )
    {
        phi = 0.0f;
    }
    else
    {
        xa = linac->cdf_rho_theta_phi[ ind - 1 ];
        xb = linac->cdf_rho_theta_phi[ ind ];
        ya = f32( ind-offset - 1 ) * linac->s_rho_theta_phi.z;
        yb = f32( ind-offset ) * linac->s_rho_theta_phi.z;
        phi = linear_interpolation( xa, ya, xb, yb, rnd );
    }

    // 3.3 Get direction vector

    f32xyz dir = { 0.0, 0.0, 0.0 };
    dir.x = sin( theta ) * cos( psi+phi );
    dir.y = sin( theta ) * sin( psi+phi );
    dir.z = cos( theta );

    // ????  dz *= -1.0 ????

    dir = fxyz_local_to_global_direction( trans, dir );

    particles->dx[id] = dir.x;                        // Direction (unit vector)
    particles->dy[id] = dir.y;                        //
    particles->dz[id] = dir.z;                        //


    // 4. Then set the mandatory field to create a new particle
    particles->tof[id] = 0.0f;                             // Time of flight
    particles->status[id] = PARTICLE_ALIVE;               // Status of the particle

    particles->level[id] = PRIMARY;                        // It is a primary particle
    particles->pname[id] = PHOTON;                         // a photon or an electron

    particles->geometry_id[id] = 0;                        // Some internal variables
    particles->next_discrete_process[id] = NO_PROCESS;     //
    particles->next_interaction_distance[id] = 0.0;        //
    particles->scatter_order[ id ] = 0;                    //


//    printf("src id %i p %f %f %f d %f %f %f E %f\n", id, pos.x, pos.y, pos.z,
//                                                         dir.x, dir.y, dir.z, E);

}


// Kernel to create new particles. This kernel will only call the host/device function
// beamlet source in order to get one new particle.
__global__ void kernel_linac_source ( ParticlesData *particles,
                                      const LinacSourceData *linac,
                                      f32matrix44 trans )
{
    // Get thread id
    const ui32 id = blockIdx.x * blockDim.x + threadIdx.x;
    if ( id >= particles->size ) return;

    // Get a new particle
    linac_source( particles, linac, trans, id );
}

//////// Class //////////////////////////////////////////////////////////

// Constructor
LinacSource::LinacSource() : GGEMSSource()
{
    // Set the name of the source
    set_name( "LinacSource" );

    // Init vars    
    m_axis_trans = make_f32matrix33( 1, 0, 0,
                                     0, 1, 0,
                                     0, 0, 1 );
    m_angle = make_f32xyz( 0.0, 0.0, 0.0 );
    m_org = make_f32xyz( 0.0, 0.0, 0.0 );
    m_model_filename = "";

    mh_params = nullptr;
    d_linac_source = nullptr;

    h_linac_source = (LinacSourceData*) malloc( sizeof(LinacSourceData) );

    h_linac_source->s_rho = 0.0;
    h_linac_source->s_rho_E = make_f32xy( 0.0, 0.0 );
    h_linac_source->s_rho_E_theta = make_f32xyz( 0.0, 0.0, 0.0 );
    h_linac_source->s_rho_theta_phi = make_f32xyz( 0.0, 0.0, 0.0 );

    h_linac_source->n_rho = 0;
    h_linac_source->n_rho_E = make_ui32xy( 0, 0 );
    h_linac_source->n_rho_E_theta = make_ui32xyz( 0, 0, 0 );
    h_linac_source->n_rho_theta_phi = make_ui32xyz( 0, 0, 0 );

    h_linac_source->cdf_rho = nullptr;
    h_linac_source->cdf_rho_E = nullptr;
    h_linac_source->cdf_rho_E_theta = nullptr;
    h_linac_source->val_rho_E_theta_max = nullptr;
    h_linac_source->cdf_rho_theta_phi = nullptr;
}

// Destructor
LinacSource::~LinacSource() {}

//========== Private ===============================================

void LinacSource::m_load_linac_model()
{    
    // Get a txt reader
    TxtReader *txt_reader = new TxtReader;

    /////////////// First read the MHD file //////////////////////

    std::string line, key;

    // Watchdog
    f32 rho_max = 0;
    f32 E_max = 0;
    f32 theta_max = 0;
    f32 phi_max = 0;

    std::string ElementDataFile = ""; std::string ObjectType = "";

    // Read file
    std::ifstream file( m_model_filename.c_str() );
    if ( !file ) {
        GGcerr << "Error, file '" << m_model_filename << "' not found!" << GGendl;
        exit_simulation();
    }

    while ( file ) {
        txt_reader->skip_comment( file );
        std::getline( file, line );

        if ( file ) {
            key = txt_reader->read_key(line);
            if ( key == "RhoMax" )               rho_max = txt_reader->read_key_f32_arg( line );
            if ( key == "EMax" )                 E_max = txt_reader->read_key_f32_arg( line );
            if ( key == "ThetaMax" )             theta_max = txt_reader->read_key_f32_arg( line );
            if ( key == "PhiMax" )               phi_max = txt_reader->read_key_f32_arg( line );
            if ( key == "ElementDataFile" )      ElementDataFile = txt_reader->read_key_string_arg( line );
            if ( key == "ObjectType" )           ObjectType = txt_reader->read_key_string_arg( line );

            if ( key == "NRho" )                 h_linac_source->n_rho = txt_reader->read_key_i32_arg( line );
            if ( key == "NRhoE" )
            {
                h_linac_source->n_rho_E = make_ui32xy( txt_reader->read_key_i32_arg_atpos( line, 0 ),
                                                       txt_reader->read_key_i32_arg_atpos( line, 1 ) );
            }
            if ( key == "NRhoETheta" )
            {
                h_linac_source->n_rho_E_theta = make_ui32xyz( txt_reader->read_key_i32_arg_atpos( line, 0 ),
                                                              txt_reader->read_key_i32_arg_atpos( line, 1 ),
                                                              txt_reader->read_key_i32_arg_atpos( line, 2 ) );
            }
            if ( key == "NRhoThetaPhi" )
            {
                h_linac_source->n_rho_theta_phi = make_ui32xyz( txt_reader->read_key_i32_arg_atpos( line, 0 ),
                                                                txt_reader->read_key_i32_arg_atpos( line, 1 ),
                                                                txt_reader->read_key_i32_arg_atpos( line, 2 ) );
            }
        }

    } // read file

    // Check the header
    if ( ObjectType != "GPILS" ) {
        GGcerr << "Linac source model header: not a virtual source model, ObjectType = " << ObjectType << " !" << GGendl;
        exit_simulation();
    }

    if ( ElementDataFile == "" ) {
        GGcerr << "Linac source model header: ElementDataFile was not specified!" << GGendl;
        exit_simulation();
    }

    if ( rho_max == 0 || E_max == 0 || theta_max == 0 || phi_max == 0 ||
         h_linac_source->n_rho == 0 ||
         h_linac_source->n_rho_E.x == 0 || h_linac_source->n_rho_E.y == 0 ||
         h_linac_source->n_rho_E_theta.x == 0 || h_linac_source->n_rho_E_theta.y == 0 || h_linac_source->n_rho_E_theta.z == 0 ||
         h_linac_source->n_rho_theta_phi.x == 0 || h_linac_source->n_rho_theta_phi.y == 0 || h_linac_source->n_rho_theta_phi.z == 0 )
    {
        GGcerr << "Missing parameter(s) in the header of Linac source model file!" << GGendl;
        exit_simulation();
    }

    // Store data and mem allocation
    h_linac_source->rho_max = rho_max;
    h_linac_source->E_max = E_max;
    h_linac_source->theta_max = theta_max;
    h_linac_source->phi_max = phi_max;

    h_linac_source->s_rho = rho_max / f32( h_linac_source->n_rho );

    h_linac_source->s_rho_E = make_f32xy( rho_max / f32( h_linac_source->n_rho_E.x ),
                                          E_max / f32( h_linac_source->n_rho_E.y ));

    h_linac_source->s_rho_E_theta = make_f32xyz( rho_max / f32( h_linac_source->n_rho_E_theta.x ),
                                                 E_max / f32( h_linac_source->n_rho_E_theta.y ),
                                                 0.0f );

    h_linac_source->s_rho_theta_phi = make_f32xyz( rho_max / f32( h_linac_source->n_rho_theta_phi.x ),
                                                   theta_max / f32( h_linac_source->n_rho_theta_phi.y ),
                                                   phi_max / f32( h_linac_source->n_rho_theta_phi.z ) );

    // Read data
    FILE *pfile = fopen(ElementDataFile.c_str(), "rb");

    // Relative path?
    if (!pfile) {
        std::string nameWithRelativePath = m_model_filename;
        i32 lastindex = nameWithRelativePath.find_last_of("/");
        nameWithRelativePath = nameWithRelativePath.substr(0, lastindex);
        nameWithRelativePath += ( "/" + ElementDataFile );

        pfile = fopen(nameWithRelativePath.c_str(), "rb");
        if ( !pfile )
        {
            GGcerr << "Error, file " << ElementDataFile << " not found " << GGendl;
            exit_simulation();
        }
    }

    // Allocation
    h_linac_source->cdf_rho = (f32*)malloc( h_linac_source->n_rho * sizeof(f32) );
    h_linac_source->cdf_rho_E = (f32*)malloc( h_linac_source->n_rho_E.x * h_linac_source->n_rho_E.y * sizeof(f32) );
    h_linac_source->cdf_rho_E_theta = (f32*)malloc( h_linac_source->n_rho_E_theta.x *
                                                    h_linac_source->n_rho_E_theta.y *
                                                    h_linac_source->n_rho_E_theta.z * sizeof(f32) );
    h_linac_source->val_rho_E_theta_max = (f32*)malloc( h_linac_source->n_rho_E_theta.x *
                                                        h_linac_source->n_rho_E_theta.y * sizeof(f32) );
    h_linac_source->cdf_rho_theta_phi = (f32*)malloc( h_linac_source->n_rho_theta_phi.x *
                                                      h_linac_source->n_rho_theta_phi.y *
                                                      h_linac_source->n_rho_theta_phi.z * sizeof(f32) );

    // Read rho
    fread( h_linac_source->cdf_rho, sizeof(f32), h_linac_source->n_rho, pfile );
    // E(rho)
    fread( h_linac_source->cdf_rho_E, sizeof(f32), h_linac_source->n_rho_E.x * h_linac_source->n_rho_E.y, pfile );
    // thetamax(rho, E)
    fread( h_linac_source->val_rho_E_theta_max, sizeof(f32), h_linac_source->n_rho_E_theta.x *
                                                             h_linac_source->n_rho_E_theta.y, pfile );
    // theta(rho, E)
    fread( h_linac_source->cdf_rho_E_theta, sizeof(f32), h_linac_source->n_rho_E_theta.x *
                                                         h_linac_source->n_rho_E_theta.y *
                                                         h_linac_source->n_rho_E_theta.z, pfile );
    // phi(rho, theta)
    fread( h_linac_source->cdf_rho_theta_phi, sizeof(f32), h_linac_source->n_rho_theta_phi.x *
                                                           h_linac_source->n_rho_theta_phi.y *
                                                           h_linac_source->n_rho_theta_phi.z, pfile );

    fclose( pfile );
}

void LinacSource::m_copy_to_gpu()
{
    ui32 n_rho = h_linac_source->n_rho;
    ui32 n_rho_E = h_linac_source->n_rho_E.x * h_linac_source->n_rho_E.y;
    ui32 n_rho_E_theta = h_linac_source->n_rho_E_theta.x *
                         h_linac_source->n_rho_E_theta.y *
                         h_linac_source->n_rho_E_theta.z;
    ui32 n_rho_theta_phi = h_linac_source->n_rho_theta_phi.x *
                           h_linac_source->n_rho_theta_phi.y *
                           h_linac_source->n_rho_theta_phi.z;

    /// First, struct allocation
    HANDLE_ERROR( cudaMalloc( (void**) &d_linac_source, sizeof( LinacSourceData ) ) );

    /// Device pointers allocation
    f32 *cdf_rho;
    HANDLE_ERROR( cudaMalloc((void**) &cdf_rho, n_rho*sizeof(f32)) );
    f32 *cdf_rho_E;
    HANDLE_ERROR( cudaMalloc((void**) &cdf_rho_E, n_rho_E*sizeof(f32)) );
    f32 *cdf_rho_E_theta;
    HANDLE_ERROR( cudaMalloc((void**) &cdf_rho_E_theta, n_rho_E_theta*sizeof(f32)) );
    f32 *val_rho_E_theta_max;
    HANDLE_ERROR( cudaMalloc((void**) &val_rho_E_theta_max, h_linac_source->n_rho_E_theta.x*h_linac_source->n_rho_E_theta.y*sizeof(f32)) );
    f32 *cdf_rho_theta_phi;
    HANDLE_ERROR( cudaMalloc((void**) &cdf_rho_theta_phi, n_rho_theta_phi*sizeof(f32)) );

    /// Copy host data to device
    HANDLE_ERROR( cudaMemcpy( cdf_rho, h_linac_source->cdf_rho,
                              n_rho*sizeof(f32), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( cdf_rho_E, h_linac_source->cdf_rho_E,
                              n_rho_E*sizeof(f32), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( cdf_rho_E_theta, h_linac_source->cdf_rho_E_theta,
                              n_rho_E_theta*sizeof(f32), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( val_rho_E_theta_max, h_linac_source->val_rho_E_theta_max,
                              h_linac_source->n_rho_E_theta.x*h_linac_source->n_rho_E_theta.y*sizeof(f32), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( cdf_rho_theta_phi, h_linac_source->cdf_rho_theta_phi,
                              n_rho_theta_phi*sizeof(f32), cudaMemcpyHostToDevice ) );

    /// Bind data to the struct
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->cdf_rho), &cdf_rho,
                              sizeof(d_linac_source->cdf_rho), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->cdf_rho_E), &cdf_rho_E,
                              sizeof(d_linac_source->cdf_rho_E), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->cdf_rho_E_theta), &cdf_rho_E_theta,
                              sizeof(d_linac_source->cdf_rho_E_theta), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->val_rho_E_theta_max), &val_rho_E_theta_max,
                              sizeof(d_linac_source->val_rho_E_theta_max), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->cdf_rho_theta_phi), &cdf_rho_theta_phi,
                              sizeof(d_linac_source->cdf_rho_theta_phi), cudaMemcpyHostToDevice ) );

    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->rho_max), &(h_linac_source->rho_max),
                              sizeof(d_linac_source->rho_max), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->E_max), &(h_linac_source->E_max),
                              sizeof(d_linac_source->E_max), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->theta_max), &(h_linac_source->theta_max),
                              sizeof(d_linac_source->theta_max), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->phi_max), &(h_linac_source->phi_max),
                              sizeof(d_linac_source->phi_max), cudaMemcpyHostToDevice ) );

    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->s_rho), &(h_linac_source->s_rho),
                              sizeof(d_linac_source->s_rho), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->s_rho_E), &(h_linac_source->s_rho_E),
                              sizeof(d_linac_source->s_rho_E), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->s_rho_E_theta), &(h_linac_source->s_rho_E_theta),
                              sizeof(d_linac_source->s_rho_E_theta), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->s_rho_theta_phi), &(h_linac_source->s_rho_theta_phi),
                              sizeof(d_linac_source->s_rho_theta_phi), cudaMemcpyHostToDevice ) );

    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->n_rho), &(h_linac_source->n_rho),
                              sizeof(d_linac_source->n_rho), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->n_rho_E), &(h_linac_source->n_rho_E),
                              sizeof(d_linac_source->n_rho_E), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->n_rho_E_theta), &(h_linac_source->n_rho_E_theta),
                              sizeof(d_linac_source->n_rho_E_theta), cudaMemcpyHostToDevice ) );
    HANDLE_ERROR( cudaMemcpy( &(d_linac_source->n_rho_theta_phi), &(h_linac_source->n_rho_theta_phi),
                              sizeof(d_linac_source->n_rho_theta_phi), cudaMemcpyHostToDevice ) );

}

//========== Setting ===============================================

// Setting the axis transformation matrix
void LinacSource::set_frame_axis( f32 m00, f32 m01, f32 m02,
                                  f32 m10, f32 m11, f32 m12,
                                  f32 m20, f32 m21, f32 m22 )
{
    m_axis_trans.m00 = m00;
    m_axis_trans.m01 = m01;
    m_axis_trans.m02 = m02;
    m_axis_trans.m10 = m10;
    m_axis_trans.m11 = m11;
    m_axis_trans.m12 = m12;
    m_axis_trans.m20 = m20;
    m_axis_trans.m21 = m21;
    m_axis_trans.m22 = m22;
}

// Setting orientation of the beamlet
void LinacSource::set_frame_rotation( f32 agx, f32 agy, f32 agz )
{
    m_angle = make_f32xyz( agx, agy, agz );
}

// Setting the distance between the beamlet plane and the isocenter
void LinacSource::set_frame_position( f32 posx, f32 posy, f32 posz )
{
    m_org = make_f32xyz( posx, posy, posz );
}

// Setting Linac source model
void LinacSource::set_model_filename( std::string filename )
{
    m_model_filename = filename;
}

//========== Getting ===============================================

f32matrix44 LinacSource::get_transformation_matrix()
{
    return m_transform;
}

//========== Update ===============================================

void LinacSource::update_rotation(f32 rx, f32 ry, f32 rz)
{
    m_angle = make_f32xyz( rx, ry, rz );

    // Recalculate the transformation matrix of the detector that map local frame to glboal frame
    TransformCalculator *trans = new TransformCalculator;
    trans->set_translation( m_org );
    trans->set_rotation( m_angle );
    trans->set_axis_transformation( m_axis_trans );
    m_transform = trans->get_transformation_matrix();
    delete trans;
}

//========= Main function ============================================

// Mandatory function, abstract from GGEMSSource. This function is called
// by GGEMS to initialize and load all necessary data into the graphic card
void LinacSource::initialize (GlobalSimulationParametersData *h_params )
{
    // Some checking
    if ( m_model_filename == "" )
    {
        GGcerr << "No filename for the Linac source model was specified!" << GGendl;
        exit_simulation();
    }

    std::string ext = m_model_filename.substr( m_model_filename.find_last_of( "." ) + 1 );
    if ( ext != "mhd" )
    {
        GGcerr << "Linac source model file must be in Meta Header Data (.mhd)!" << GGendl;
        exit_simulation();
    }

    // Store global parameters: params are provided by GGEMS and are used to
    // know different information about the simulation. For example if the targeted
    // device is a CPU or a GPU.
    mh_params = h_params;

    // Read and load data
    m_load_linac_model();
    m_copy_to_gpu();

    // Compute the transformation matrix (Beamlet plane is set along the x-axis (angle 0))
    TransformCalculator *trans = new TransformCalculator;
    trans->set_translation( m_org );
    trans->set_rotation( m_angle );
    trans->set_axis_transformation( m_axis_trans );
    m_transform = trans->get_transformation_matrix();
    delete trans;

    // Some verbose if required
    if ( h_params->display_memory_usage )
    {
        ui32 mem = 4 * ( h_linac_source->n_rho +
                         h_linac_source->n_rho_E.x + h_linac_source->n_rho_E.y +
                         h_linac_source->n_rho_E_theta.x + h_linac_source->n_rho_E_theta.y + h_linac_source->n_rho_E_theta.z +
                         h_linac_source->n_rho_theta_phi.x + h_linac_source->n_rho_theta_phi.y + h_linac_source->n_rho_theta_phi.z );
        GGcout_mem("Linac source", mem);
    }

}

// Mandatory function, abstract from GGEMSSource. This function is called
// by GGEMS to fill particle buffer of new fresh particles, which is the role
// of any source.
void LinacSource::get_primaries_generator (ParticlesData *d_particles )
{

    // Defined threads and grid
    dim3 threads, grid;
    threads.x = mh_params->gpu_block_size;
    grid.x = ( mh_params->size_of_particles_batch + mh_params->gpu_block_size - 1 ) / mh_params->gpu_block_size;

    // Call GPU kernel of a point source that get fill the complete particle buffer. In this case data
    // from device (GPU) is passed to the kernel (particles.data_d).
    kernel_linac_source<<<grid, threads>>>( d_particles, d_linac_source,
                                            m_transform );
    cuda_error_check( "Error ", " Kernel_beamlet_source" );
    cudaDeviceSynchronize();

}

#endif

















